#include <include/Log.hh>

#include <unistd.h>

namespace dprobe {
namespace log {

std::ostream& operator<<(std::ostream& out, Level lev) {
    switch(lev) {
    case Level::fatal  : out << "F"; break;
    case Level::error  : out << "E"; break;
    case Level::warn   : out << "W"; break;
    case Level::info   : out << "I"; break;
    case Level::debug  : out << "D"; break;
    }
    return out;
}

Record::~Record() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    {
        std::lock_guard<std::mutex> lg(Logger::get().mutex());
        std::cout << "[" << getpid() << " " << std::put_time(&tm, "%e.%m.%Y %H:%M:%S") << "] " << level_
            << " " << str() << "\n";
        std::cout.flush();
    }
}

}
}
