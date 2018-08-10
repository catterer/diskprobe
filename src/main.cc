#include <include/Dispatcher.hh>

#include <iostream>

int main(int argc, char** argv) try {
    dprobe::Dispatcher cfg(argc, argv);
    cfg.loop();
    return 0;
} catch(const std::exception& x) {
    std::cerr << x.what() << "\n";
    return EXIT_FAILURE;
}
