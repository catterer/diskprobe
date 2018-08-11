#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

#define Log(lev) \
    for (int i = 1; i; i = 0) \
        Record(lev)

#define GET_MACRO(_1,_2,NAME,...) NAME
#define NLog(...) GET_MACRO(__VA_ARGS__, NLog2, NLog1)(__VA_ARGS__)

#define NLog2(l, n) Log(#l) << n << ": "
#define NLog1(l) NLog2(l, name())

namespace dprobe {

class Record: public std::stringstream {
public:
    using std::stringstream::stringstream;

    Record(const char* l): level_{l} {}

    virtual ~Record() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << "[" << getpid() << " " << std::put_time(&tm, "%e.%m.%Y %H:%M:%S") << "] " << *level_ << " " << str() << "\n";
    }

private:
    const char* level_{};
};

}
