#pragma once
#include <include/Probe.hh>
#include <include/Queue.hh>
#include <include/Log.hh>
#include <chrono>
#include <stdexcept>
#include <stdint.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>

namespace dprobe {

namespace po = boost::program_options;

class Dispatcher {
public:
    Dispatcher(int argc, char** argv);
    void loop();

private:
    void process(std::shared_ptr<message::AbstractMessage>);

    Queue queue_;
    std::map<std::string, std::unique_ptr<probe::AbstractProbe>> probes_;
    uint32_t max_sampling_rate_ms_;
};

Dispatcher::Dispatcher(int argc, char** argv) {
    std::string config_file;
    po::options_description desc("Allowed options");
    desc.add_options()

        ("help",
            "produce help message")

        ("config,c",
            po::value(&config_file),
            "path to config file")

        ("max-sampling-rate,s", 
            po::value(&max_sampling_rate_ms_)->default_value(100),
            "how often does main thread takes looks at the clock, msec");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (config_file.empty())
        throw std::invalid_argument("config file not specified");

    boost::property_tree::ptree tree;
    boost::property_tree::ini_parser::read_ini(config_file, tree);

    for (const auto& p: tree) {
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
