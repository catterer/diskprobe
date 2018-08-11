#pragma once
#include <include/Channel.hh>

#include <include/chrono_aliases.hh>
#include <thread>
#include <iostream>
#include <functional>
#include <boost/property_tree/ptree.hpp>

namespace dprobe {
namespace probe {

using boost::property_tree::ptree;

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
    const std::string name_;
    const ptree ptree_;
    Queue&      queue_;
    std::thread thread_;
};

class HeartbeatingProbe: public AbstractProbe {
public:
    HeartbeatingProbe(const std::string& name, const ptree& pt, Queue& q):
        AbstractProbe(name, pt, q),
        period_{pt.get("heartbeat_period_ms", 1000U)}
    { }

    void check(time_point now) override {
        if (now < last_heartbeat_)
            return;
        if (now - last_heartbeat_ > period_ * 1.5) {
            if (!down_) {
                std::cout << name() << ": DOWN" << "\n";
                down_ = true;
            }
        }
    }

    auto heartbeat_period() const -> milliseconds { return period_; }

private:
    bool            down_{true};
    time_point      last_heartbeat_;
    milliseconds    period_;
};

class FileWriter: public HeartbeatingProbe {
public:
    using HeartbeatingProbe::HeartbeatingProbe;

    void loop(Channel&, const ptree&);
};
}

}
