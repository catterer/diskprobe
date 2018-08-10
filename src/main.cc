#include <include/Config.hh>

#include <iostream>

int main(int argc, char** argv) try {
    dprobe::Config cfg(argc, argv);
    return 0;
} catch(const std::exception& x) {
    std::cerr << x.what() << "\n";
    return EXIT_FAILURE;
}
