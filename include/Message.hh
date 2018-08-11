#pragma once
#include <string>

namespace dprobe {

namespace message {

class AbstractMessage {
public:
    AbstractMessage() = default;
    virtual ~AbstractMessage() = default;

    auto sender() const -> const std::string& { return sender_; }
    void sender(std::string sender) { sender_ = sender; }

protected:
    std::string sender_;
};

class Error: public AbstractMessage {
public:
    Error(const std::string& description):
        description_{description}
    {}

    auto description() const -> const std::string& { return description_; }

private:
    std::string description_;
};

class Abort: public AbstractMessage {
public:
    Abort(const std::string& what):
        what_{what}
    {}

    auto what() const -> const std::string& { return what_; }

private:
    std::string what_;
};

class Heartbeat: public AbstractMessage {
public:
    Heartbeat() = default;
};

};

}
