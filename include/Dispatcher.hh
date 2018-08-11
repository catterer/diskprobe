#pragma once
#include <include/Probe.hh>
#include <include/Queue.hh>
#include <include/Log.hh>
#include <chrono>
#include <stdexcept>
#include <stdint.h>

namespace dprobe {

class Dispatcher {
public:
    Dispatcher(uint32_t max_sampling_rate_ms, const probe::Options& root);
    void loop();

private:
    void process(std::shared_ptr<message::AbstractMessage>);

    Queue queue_;
    std::map<std::string, std::unique_ptr<probe::AbstractProbe>> probes_;
    uint32_t max_sampling_rate_ms_;
};

Dispatcher::Dispatcher(uint32_t max_sampling_rate_ms, const probe::Options& root):
    max_sampling_rate_ms_{max_sampling_rate_ms}
{
    for (const auto& p: root) {
        if (probes_.count(p.first))
            throw std::runtime_error("probe names duplication");
        probes_.emplace(p.first, probe::factory(p.first, p.second, queue_));
    }
}

void Dispatcher::loop() {
    for (auto& pb: probes_)
        pb.second->start();

    while (not probes_.empty()) {
        auto m = queue_.pop_for(milliseconds(max_sampling_rate_ms_));
        process(std::move(m));

        auto now = time_now();
        for (auto& pb: probes_)
            pb.second->check(now);
    }
}

void Dispatcher::process(std::shared_ptr<message::AbstractMessage> some_msg) {
    if (!some_msg)
        return;

    {
        auto ab = std::dynamic_pointer_cast<message::Abort>(some_msg);
        if (ab) {
            NLog(fatal, ab->sender()) << "Abort";
            probes_.erase(ab->sender());
            return;
        }
    }

    auto pi = probes_.find(some_msg->sender());
    if (pi != probes_.end())
        pi->second->processMessage(some_msg);
}

}
