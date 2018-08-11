#include <include/Probe.hh>
#include <include/Log.hh>

#include <vector>
#include <list>
#include <cstring>
#include <unistd.h>

namespace dprobe {
namespace probe {

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

    auto script = options().get("on_reappear", std::string());
    if (!script.empty())
        forkexec(script);
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
