#pragma once
#include <thread>
#include <boost/property_tree/ptree.hpp>

namespace dprobe {

using boost::property_tree::ptree;

class Probe {
public:

    Probe(const std::string& name, const ptree&):
        name_{name} {}

    auto name() const -> const std::string& { return name_; }
    void start() {}
    void join() {}

private:
    std::string name_;
    std::thread thread_;
};

}
