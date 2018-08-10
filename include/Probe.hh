#pragma once
#include <include/Channel.hh>

#include <chrono>
#include <thread>
#include <functional>
#include <boost/property_tree/ptree.hpp>

namespace dprobe {
namespace probe {

using boost::property_tree::ptree;
using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

class AbstractProbe;;
auto factory(const std::string& probename, const ptree&, Queue&) -> std::unique_ptr<AbstractProbe>;

class AbstractProbe {
public:
    AbstractProbe(const std::string& name, const ptree& pt, Queue& q):
        name_{name}, ptree_{pt}, queue_{q} {}
    virtual ~AbstractProbe() { if (thread_.joinable()) thread_.join(); }

    auto name() const -> const std::string& { return name_; }
    void start() {
        thread_ = std::thread(
            [this] () {
                Channel chan(name_, queue_);
                try {
                    loop(chan, ptree_);
                } catch (const std::exception& x) {
                    chan.send<message::Abort>(x.what());
                }
            });
    }

    virtual void check(time_point now) = 0;
    virtual void loop(Channel&, const ptree&) = 0;

private:
    time_point  last_heartbeat_;
    const std::string name_;
    const ptree ptree_;
    Queue&      queue_;
    std::thread thread_;
};

class FileWriter: public AbstractProbe {
public:
    using AbstractProbe::AbstractProbe;

    void check(time_point now) { /* TODO */; }
    void loop(Channel&, const ptree&);
};
}

}
