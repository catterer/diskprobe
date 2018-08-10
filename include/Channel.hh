#pragma once
#include <include/Queue.hh>

namespace dprobe {

class Channel {
public:
    Channel(const std::string& sender, Queue& q):
        sender_{sender}, queue_{q} {}

    template<typename MSGTYPE, typename ...Args>
    void send(Args&& ...args) {
        auto msg = std::make_unique<MSGTYPE>(std::forward<Args>(args)...);
        msg->sender(sender_);
        queue_.push(std::move(msg));
    }

private:
    std::string sender_;
    Queue&      queue_;
};

}
