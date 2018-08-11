#include <include/Probe.hh>
#include <include/Log.hh>

#include <boost/filesystem/path.hpp>
#include <vector>
#include <cstring>

namespace dprobe {
namespace probe {

using Path = boost::filesystem::path;

void AbstractProbe::forkexec(const std::string& script) {
    auto to_tok = std::shared_ptr<char>(strdup(script.c_str()), [] (char* p) { free(p); });
    std::vector<std::string> toks;
    for (auto tok = strtok(to_tok.get(), " "); tok; tok = strtok(NULL, " "))
        toks.emplace_back(tok);

    if (toks.empty())
        throw std::runtime_error("Invalid path");

    Path path(toks[0]);
    NLog(warn) << "got path " << path;
}

auto factory(const std::string& probename, const Options& pt, Queue& queue)
    -> std::unique_ptr<AbstractProbe>
{
    // TODO
    return std::make_unique<FaultyHeartbeat>(probename, pt, queue);
}

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
    chan.send<message::Heartbeat>();
}

void HeartbeatingProbe::check(time_point now) {
    if (now < last_heartbeat_)
        return;
    if (now - last_heartbeat_ > period() * 1.5) {
        if (!down_) {
            down_ = true;
            onDown();
        }
    }
}

void HeartbeatingProbe::processMessage(std::shared_ptr<message::AbstractMessage> some_msg) {
    {
        auto hb = std::dynamic_pointer_cast<message::Heartbeat>(some_msg);
        if (hb) {
            NLog(debug) << "Heartbeat received";
            last_heartbeat_ = time_now();
            if (down_) {
                down_ = false;
                onUp();
            }
            return;
        }
    }
}

void HeartbeatingProbe::onUp() {
    NLog(warn) << "UP";
}

void HeartbeatingProbe::onDown() {
    NLog(error) << "DOWN";
    auto script = options().get("on_timeout", std::string());
    if (!script.empty())
        forkexec(script);
}

void FaultyHeartbeat::iteration(Channel& chan) {
    if (rand() % 100 < options().get("fault_probability", 50U))
        return;
    HeartbeatingProbe::iteration(chan);
}


}
}
