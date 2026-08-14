#pragma once

#include "sysscope/core/metric.hpp"
#include <mutex>
#include <condition_variable>
#include <deque>
#include <optional>
#include <cstddef>

namespace sysscope::telemetry {

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(size_t capacity = 8192)
        : capacity_(capacity > 0 ? capacity : 8192) {}

    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= capacity_) {
            queue_.pop_front(); // Drop oldest element on overflow
            dropped_count_++;
        }
        queue_.push_back(std::move(item));
        cv_.notify_one();
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop_front();
        return item;
    }

    std::optional<T> wait_and_pop(std::chrono::milliseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, timeout, [this] { return !queue_.empty() || shutdown_; })) {
            return std::nullopt;
        }
        if (queue_.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop_front();
        return item;
    }

    void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }

    [[nodiscard]] size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    [[nodiscard]] uint64_t dropped_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return dropped_count_;
    }

private:
    size_t capacity_;
    std::deque<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_{false};
    uint64_t dropped_count_{0};
};

} // namespace sysscope::telemetry
