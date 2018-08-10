#pragma once
#include <include/Message.hh>
#include <memory>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace dprobe {

using MsgPtr = std::unique_ptr<message::AbstractMessage>;

class Queue: private std::queue<MsgPtr> {
public:
    Queue() = default;

    auto pop_for(std::chrono::milliseconds ms) -> MsgPtr {
        std::unique_lock<std::mutex> lk(mut_);
        if (!condvar_.wait_for(lk, ms, [this] () { return !empty(); }))
            return {};

        auto msg = std::move(front());
        std::queue<MsgPtr>::pop();
        lk.unlock();

        return msg;
    }

    auto push(MsgPtr&& msg) {
        {
            std::lock_guard<std::mutex> g(mut_);
            std::queue<MsgPtr>::push(std::move(msg));
        }

        condvar_.notify_one();
    }

private:
    std::mutex mut_;
    std::condition_variable condvar_;
};

}
