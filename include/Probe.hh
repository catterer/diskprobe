#pragma once
#include <include/Channel.hh>
#include <include/Utils.hh>
#include <include/chrono_aliases.hh>

#include <map>
#include <thread>
#include <iostream>
#include <functional>
#include <boost/property_tree/ptree.hpp>

namespace dprobe {
namespace probe {

using Options = boost::property_tree::ptree;

class AbstractProbe;

class ProbeFactory: public Singleton<ProbeFactory> {
public:
    ProbeFactory();

    auto build(const std::string& probename, const Options&, Queue&) -> std::unique_ptr<AbstractProbe>;

private:
    using ProbeCreator = std::function<std::unique_ptr<AbstractProbe>(const std::string&, const Options&, Queue&)>;
    std::map<std::string, ProbeCreator> typemap_;
};

class AbstractProbe {
public:
    AbstractProbe(const std::string& name, const Options&, Queue&);
    virtual ~AbstractProbe() { if (thread_.joinable()) thread_.join(); }

    void start();

    auto name() const -> const std::string& { return name_; }
    auto period() const -> milliseconds { return period_; }
    auto timeout() const -> milliseconds { return period_ * 2; }
    auto options() const -> const Options& { return options_; }

    virtual void check(time_point now) = 0;
    virtual void processMessage(std::shared_ptr<message::AbstractMessage>) = 0;

protected:
    void forkexec(std::string script);

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

    virtual void onUp();
    virtual void onDown(const std::string& err);

    virtual int runTask(Channel&) = 0;

protected:
    void iteration(Channel&) override;

private:
    void fail(const std::string& description);
    void callScriptIfNeeded(std::string param_name);

    bool                                is_down_{false};
    time_point                          last_heartbeat_{time_now()};
    std::map<std::string, time_point>   last_script_call{};
};

class FaultyHeartbeat: public HeartbeatingProbe {
public:
    using HeartbeatingProbe::HeartbeatingProbe;

private:
    void iteration(Channel&) override;
};

class FileWriter: public HeartbeatingProbe {
public:
    FileWriter(const std::string& name, const Options&, Queue&);

private:
    int runTask(Channel&) override;

    const std::string filename_;
};

class FileReader: public HeartbeatingProbe {
public:
    FileReader(const std::string& name, const Options&, Queue&);

private:
    int runTask(Channel&) override;

    const std::string filename_;
};

}
}
