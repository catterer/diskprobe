#include <include/Probe.hh>
#include <include/Log.hh>

namespace dprobe {
namespace probe {

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
            NLog(error) << "DOWN";
            down_ = true;
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
                NLog(warn) << "UP";
                down_ = false;
            }
            return;
        }
    }
}

void FaultyHeartbeat::iteration(Channel& chan) {
    if (rand() % 100 < options().get("fault_probability", 50U))
        return;
    HeartbeatingProbe::iteration(chan);
}


}
}
