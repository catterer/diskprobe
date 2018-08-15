#include <include/Probe.hh>
#include <include/Log.hh>

#include <vector>
#include <numeric>
#include <list>
#include <cstring>
#include <unistd.h>
#include <stdio.h>

namespace dprobe {
namespace probe {

ProbeFactory::ProbeFactory() {
#define MAP(keyword, type) \
    typemap_[keyword] = [] (const auto& name, const Options& opts, Queue& queue) { return std::make_unique<type>(name, opts, queue); };
    MAP("fail_randomly_with_probability",   FaultyHeartbeat);
    MAP("write_smth_to_file",               FileWriter);
    MAP("read_smth_from_file",              FileReader);
}

template<typename CONT, typename CB>
static auto flatten(const CONT& cont, CB cb)
    -> std::string
{
    std::stringstream out;
    for (auto i = cont.cbegin(); i != cont.cend(); i++)
        cb(*i, out);
    return out.str();
}

auto ProbeFactory::build(const std::string& probename, const Options& opts, Queue& que)
    -> std::unique_ptr<AbstractProbe>
{
    std::list<std::string> matches;

    for (const auto& p: typemap_)
        if (opts.count(p.first))
            matches.emplace_back(p.first);
    
    if (matches.empty()) {
        NLog(error, probename) << "unknown probe type; available: "
            << flatten(typemap_, [] (const auto& p, auto& out) { out << p.first << ","; });
        throw std::invalid_argument(probename);
    }

    if (matches.size() > 1) {
        NLog(error, probename) << "conflicting keywords: "
            << flatten(matches, [] (const auto& m, auto& out) { out << m << ","; });
        throw std::invalid_argument(probename);
    }

    return typemap_[*matches.begin()](probename, opts, que);
}

void AbstractProbe::forkexec(std::string script) {
    NLog(warn) << "executing '" << script << "'";

    auto pid = fork();
    if (pid < 0)
        throw std::runtime_error(strerror(errno));

    if (pid)
        return;

    execlp("bash", "bash", "-c", script.c_str(), nullptr);
    NLog(error) << "child process: " << strerror(errno) << " (" << script << ")";
    abort();
}

auto factory(const std::string& probename, const Options& pt, Queue& queue)
    -> std::unique_ptr<AbstractProbe>
{
    // TODO
    return std::make_unique<FaultyHeartbeat>(probename, pt, queue);
}

AbstractProbe::AbstractProbe(const std::string& name, const Options& pt, Queue& q):
    name_{name},
    options_{pt},
    queue_{q},
    period_{options_.get("period_ms", 1000U)}
{ }

void AbstractProbe::start() {
    thread_ = std::thread(
        [this] () {
            Channel chan(name(), queue_);
            try {
                loop(chan);
            } catch (const std::exception& x) {
                NLog(error) << "unhandled exception " << x.what();
                chan.send<message::Abort>(x.what());
            }
        });
}

void AbstractProbe::loop(Channel& chan) {
    for (;;) {
        std::this_thread::sleep_for(period());
        iteration(chan);
    }
}

void HeartbeatingProbe::iteration(Channel& chan) {
    auto started = time_now();
    int rc = runTask(chan);
    auto diff = time_now() - started;

    if (rc)
        return;

    if (diff >= timeout() - period()) {
        NLog(error) << "took too much time to run task: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " msec";
        return;
    }

    chan.send<message::Heartbeat>();
}

void HeartbeatingProbe::check(time_point now) {
    if (now < last_heartbeat_)
        return;
    if (now - last_heartbeat_ >= timeout())
        return fail("no heartbeat within period");
}

void HeartbeatingProbe::processMessage(std::shared_ptr<message::AbstractMessage> some_msg) {
    auto er = std::dynamic_pointer_cast<message::Error>(some_msg);
    if (er)
        return fail(er->description());

    auto hb = std::dynamic_pointer_cast<message::Heartbeat>(some_msg);
    if (hb) {
        NLog(debug) << "Heartbeat received";
        last_heartbeat_ = time_now();
        if (is_down_) {
            is_down_ = false;
            onUp();
        }
        return;
    }
}

void HeartbeatingProbe::fail(const std::string& err) {
    if (!is_down_) {
        is_down_ = true;
        onDown(err);
    }
}

void HeartbeatingProbe::callScriptIfNeeded(std::string param_name) {
    auto script = options().get(param_name, std::string());

    if (script.empty())
        return;

    auto param_between = std::string("minimal_period_ms_between__") + param_name;
    auto period_between = options().get(param_between, 0U);

    const auto now = time_now();

    if (period_between
            and last_script_call.count(param_name)
            and (now - last_script_call[param_name] < milliseconds(period_between)))
    {
        NLog(warn) << "Calling suppressed by " << param_between << ": '" << script <<"'";
        return;
    }

    last_script_call[param_name] = now;

    forkexec(script);
}

void HeartbeatingProbe::onUp() {
    NLog(warn) << "UP";
    callScriptIfNeeded("on_repair");
}

void HeartbeatingProbe::onDown(const std::string& err) {
    NLog(error) << "DOWN: " << err;
    callScriptIfNeeded("on_failure");
}

int FaultyHeartbeat::runTask(Channel& chan) {
    if ((unsigned)(rand() % 100) < options().get("fail_randomly_with_probability", 20U))
        std::this_thread::sleep_for(timeout());
    return 0;
}

FileWriter::FileWriter(const std::string& name, const Options& opts, Queue& que):
    HeartbeatingProbe(name, opts, que),
    filename_(options().get<std::string>("write_smth_to_file"))
{ }

int FileWriter::runTask(Channel& chan) {
    auto f = std::shared_ptr<FILE>(fopen(filename_.c_str(), "w"), [] (FILE* f) { if (f) fclose(f); });
    if (!f.get())
        return chan.send<message::Error>(strerror(errno)), -1;
    char byte{};
    auto rc = fwrite(&byte, 1, 1, f.get());
    if (rc != 1 or fflush(f.get()) < 0)
        return chan.send<message::Error>(strerror(errno)), -1;

    return 0;
}

FileReader::FileReader(const std::string& name, const Options& opts, Queue& que):
    HeartbeatingProbe(name, opts, que),
    filename_(options().get<std::string>("read_smth_from_file"))
{ }

int FileReader::runTask(Channel& chan) {
    auto f = std::shared_ptr<FILE>(fopen(filename_.c_str(), "r"), [] (FILE* f) { if (f) fclose(f); });
    if (!f.get())
        return chan.send<message::Error>(strerror(errno)), -1;

    char byte{};
    auto rc = fread(&byte, 1, 1, f.get());
    if (rc != 1)
        return chan.send<message::Error>(strerror(errno)), -1;

    return 0;
}


}
}
