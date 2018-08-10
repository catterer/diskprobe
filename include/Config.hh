#pragma once
#include <include/Probe.hh>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>

namespace dprobe {

namespace po = boost::program_options;

class Config {
public:
    Config(int argc, char** argv);
    void loop();

private:
    std::map<std::string, Probe> probes_;
};

Config::Config(int argc, char** argv) {
    std::string config_file;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config,c", po::value(&config_file), "path to config file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (config_file.empty())
        throw std::invalid_argument("config file not specified");

    boost::property_tree::ptree tree;
    boost::property_tree::ini_parser::read_ini(config_file, tree);

    for (const auto& p: tree)
        probes_.emplace(p.first, Probe(p.first, p.second));
}

void Config::loop() {
    for (auto& pb: probes_)
        pb.second.start();

    for (auto& pb: probes_)
        pb.second.join();
}

}
