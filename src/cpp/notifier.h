#pragma once

#include <this_thread.h>
#include <lua-helpers.h>

#include <mutex>
#include <condition_variable>

namespace effil {

class Notifier {
public:
    typedef std::function<void()> InterruptChecker;

    Notifier() : notified_(false) {}

    void notify() {
        std::unique_lock<std::mutex> lock(mutex_);
        notified_ = true;
        cv_.notify_all();
    }

    void interrupt() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_all();
    }

    void wait() {
        this_thread::setNotifier(this);
        ScopeGuard scope([](){
            this_thread::setNotifier(nullptr);
        });

        std::unique_lock<std::mutex> lock(mutex_);
        while (!notified_) {
            cv_.wait(lock);
            this_thread::interruptionPoint();
        }
    }

    template <typename T>
    bool waitFor(T period) {
        if (period == std::chrono::seconds(0) || notified_)
            return notified_;

        this_thread::setNotifier(this);
        ScopeGuard scope([](){
            this_thread::setNotifier(nullptr);
        });

        std::unique_lock<std::mutex> lock(mutex_);
        while (cv_.wait_for(lock, period) != std::cv_status::timeout && !notified_) {
            this_thread::interruptionPoint();
        }
        return notified_;
    }

    void reset() {
        notified_ = false;
    }

private:
    bool notified_;
    std::mutex mutex_;
    std::condition_variable cv_;

private:
    Notifier(Notifier& ) = delete;
};

} // namespace effil
