#pragma once

#include <limits.h>

namespace dprobe {

class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

template<typename T>
class Singleton: NonCopyable {
public:
    static auto get() -> T& {
        static T instance_;
        return instance_;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

static inline uint64_t str_to_u64(std::string s) {
    if (s.empty())
        throw std::runtime_error("empty argument value");

    uint64_t multi = 1;
    auto last = *s.rbegin();
    if (!isdigit(last)) {
        switch (last) {
        case 'T':   multi *= 1024;
                    // fall through
        case 'G':   multi *= 1024;
                    // fall through
        case 'M':   multi *= 1024;
                    // fall through
        case 'K':   multi *= 1024; break;
        default: throw std::runtime_error("invalid argument value");
        }
        s.pop_back();
    }

    char* e;
    auto res = strtoull(s.c_str(), &e, 10);

    if (*e or res == ULLONG_MAX)
        throw std::runtime_error("invalid argument value");

    return res*multi;
}

}
