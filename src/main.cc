#include <include/Dispatcher.hh>

#include <iostream>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/program_options.hpp>


int main(int argc, char** argv) try {
    using namespace dprobe;
    namespace po = boost::program_options;

    std::string config_file;
    po::options_description desc("Allowed options");
    uint32_t log_level = UINT32_MAX;
    uint32_t max_sampling_rate_ms;

    desc.add_options()

        ("help",
            "produce help message")

        ("config,c",
            po::value(&config_file),
            "path to config file")

        ("log-level,l",
            po::value(&log_level),
            "0-4, 0: only fatal, 4: everything")

        ("max-sampling-rate,s", 
            po::value(&max_sampling_rate_ms)->default_value(100),
            "how often does main thread takes looks at the clock, msec");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        exit(EXIT_SUCCESS);
    }

    if (config_file.empty())
        throw std::invalid_argument("config file not specified");

    if (log_level != UINT32_MAX)
        log::Logger::get().filter((log::Level) log_level);

    probe::Options root;
    boost::property_tree::ini_parser::read_ini(config_file, root);

    dprobe::Dispatcher cfg(max_sampling_rate_ms, root);
    cfg.loop();
    return 0;
} catch(const std::exception& x) {
    std::cerr << x.what() << "\n";
    return EXIT_FAILURE;
}
