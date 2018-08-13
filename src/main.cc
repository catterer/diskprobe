#include <include/Dispatcher.hh>
#include <include/Utils.hh>

#include <stdio.h>
#include <argp.h>
#include <iostream>
#include <boost/property_tree/ini_parser.hpp>

const char *argp_program_version = "dprobe.0.1";
const char *argp_program_bug_address = "harvos.arsen@gmail.com";
static char doc[] = "dprobe - a resource accessibility checker";
static const char* args_doc = "";

struct ProgOptions {
    std::string config{};
    std::string out_file{};
    uint64_t log_level{2};
    uint64_t max_sampling_rate_ms{100};
};

static struct argp_option optdesc[] = {
    {"config",'c',                  "PATH", 0, "path to config file", 0},
    {"out-file",'o',                "PATH", 0, "path to log file", 0},
    {"log-level",'l',               "NUMBER", 0, "0-4, 0: only fatal, 4: everything", 0},
    {"max-sampling-rate-ms",'s',    "NUMBER", 0, "how often does main thread takes looks at the clock, msec", 0},
    {0, 0, 0, 0, 0, 0}
};

auto parse_opt (int key, char *arg, struct argp_state *state) -> error_t
{
    auto opts = reinterpret_cast<ProgOptions*>(state->input);

    switch (key) {
    case 'c':   opts->config = arg; break;
    case 'o':   opts->out_file = arg; break;
    case 'l':   opts->log_level = dprobe::str_to_u64(arg); break;
    case 's':   opts->max_sampling_rate_ms = dprobe::str_to_u64(arg); break;
    default: return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

int main(int argc, char** argv) try {
    using namespace dprobe;

    ProgOptions opts;
    struct argp argp = { optdesc, parse_opt, args_doc, doc, NULL, NULL, NULL };
    argp_parse (&argp, argc, argv, 0, 0, reinterpret_cast<void*>(&opts));

    if (!opts.out_file.empty()) {
        if (freopen(opts.out_file.c_str(), "w+", stdout) == NULL)
            throw std::runtime_error("Failed to open logfile");
    }

    if (opts.config.empty())
        throw std::invalid_argument("config file not specified");

    log::Logger::get().filter((log::Level) opts.log_level);

    probe::Options root;
    boost::property_tree::ini_parser::read_ini(opts.config, root);

    dprobe::Dispatcher cfg(opts.max_sampling_rate_ms, root);
    cfg.loop();
    return 0;
} catch(const std::exception& x) {
    std::cerr << x.what() << "\n";
    return EXIT_FAILURE;
}
