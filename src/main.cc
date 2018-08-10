#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>

namespace dprobe {
namespace po = boost::program_options;

struct config {
    config(int argc, char** argv);

    std::string config_file;
};

config::config(int argc, char** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("config,c", po::value(&config_file), "path to config file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (config_file.empty())
        throw std::invalid_argument("config file not specified");

    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(config_file, pt);
    for (const auto& p: pt)
        std::cout << p.first << "\n";
//    std::cout << pt.get<std::string>("Section1.Value1") << std::endl;
}
}

int main(int argc, char** argv) try {
    dprobe::config cfg(argc, argv);
    return 0;
} catch(const std::exception& x) {
    std::cerr << x.what() << "\n";
    return EXIT_FAILURE;
}
