#pragma once
#include <include/Channel.hh>

#include <include/chrono_aliases.hh>
#include <thread>
#include <iostream>
#include <functional>
#include <boost/property_tree/ptree.hpp>

namespace dprobe {
namespace probe {

using Options = boost::property_tree::ptree;

class AbstractProbe;;
auto factory(const std::string& probename, const Options&, Queue&) -> std::unique_ptr<AbstractProbe>;

class AbstractProbe {
public:
    AbstractProbe(const std::string& name, const Options& pt, Queue& q):
        name_{name},
        options_{pt},
        queue_{q},
        period_{options_.get("heartbeat_period_ms", 1000U)}
    { }

    virtual ~AbstractProbe() { if (thread_.joinable()) thread_.join(); }

    void start();

    auto name() const -> const std::string& { return name_; }
    auto period() const -> milliseconds { return period_; }
    auto options() const -> const Options& { return options_; }

    virtual void check(time_point now) = 0;
    virtual void processMessage(std::shared_ptr<message::AbstractMessage>) = 0;

private:
    virtual void iteration(Channel&) = 0;
    void loop(Channel&);

    const std::string   name_;
    const Options       options_;
    Queue&              queue_;
    const milliseconds  period_;
    std::thread         thread_;
};

class HeartbeatingProbe: public AbstractProbe {
public:
    using AbstractProbe::AbstractProbe;

    void check(time_point now) override;
    void processMessage(std::shared_ptr<message::AbstractMessage>) override;

protected:
    void iteration(Channel&) override;

private:

    bool            down_{true};
    time_point      last_heartbeat_;
};

class FaultyHeartbeat: public HeartbeatingProbe {
public:
    using HeartbeatingProbe::HeartbeatingProbe;

private:
    void iteration(Channel&) override;
};
}

}
