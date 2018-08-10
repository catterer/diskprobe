#pragma once
#include <string>

namespace dprobe {

namespace message {

class AbstractMessage {
public:
    enum class Type {
        Heartbeat,
        Abort
    };

    AbstractMessage(Type type):
        type_{type}
    {}
    virtual ~AbstractMessage() = default;

    auto sender() const -> const std::string& { return sender_; }
    void sender(std::string sender) { sender_ = sender; }
    auto type() const -> Type { return type_; }

protected:
    std::string sender_;
    Type        type_;
};

class Abort: public AbstractMessage {
public:
    Abort(const std::string& what):
        AbstractMessage(Type::Abort),
        what_{what}
    {}

private:
    std::string what_;
};

class Heartbeat: public AbstractMessage {
public:
    Heartbeat():
        AbstractMessage(Type::Heartbeat)
    {}
};

};

}
