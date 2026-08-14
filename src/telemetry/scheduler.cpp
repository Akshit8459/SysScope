#include "sysscope/telemetry/scheduler.hpp"
#include <algorithm>

namespace sysscope::telemetry {

TelemetryScheduler::TelemetryScheduler() = default;

TelemetryScheduler::~TelemetryScheduler() {
    stop();
}

void TelemetryScheduler::add_task(std::string name, std::chrono::milliseconds interval, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    tasks_.push_back(SamplingTask{
        std::move(name),
        interval,
        now + interval,
        std::move(callback)
    });
}

void TelemetryScheduler::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) return;
    running_ = true;
    worker_thread_ = std::thread(&TelemetryScheduler::run_loop, this);
}

void TelemetryScheduler::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) return;
        running_ = false;
    }
    cv_.notify_all();
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void TelemetryScheduler::run_loop() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            cv_.wait_for(lock, std::chrono::milliseconds(100), [this] { return !running_; });
            continue;
        }

        // Find nearest deadline
        auto min_it = std::min_element(tasks_.begin(), tasks_.end(),
            [](const SamplingTask& a, const SamplingTask& b) {
                return a.next_deadline < b.next_deadline;
            });

        auto now = std::chrono::steady_clock::now();
        if (min_it->next_deadline > now) {
            cv_.wait_until(lock, min_it->next_deadline, [this] { return !running_; });
            if (!running_) break;
        }

        // Execute all ready tasks
        now = std::chrono::steady_clock::now();
        for (auto& task : tasks_) {
            if (task.next_deadline <= now) {
                if (task.callback) {
                    // Unlock while invoking callback to avoid holding mutex during execution
                    lock.unlock();
                    task.callback();
                    lock.lock();
                }
                // Deadline-based scheduling: next_deadline = previous_deadline + interval
                task.next_deadline = task.next_deadline + task.interval;
                if (task.next_deadline < now) {
                    task.next_deadline = now + task.interval; // Catch up if delayed
                }
            }
        }
    }
}

} // namespace sysscope::telemetry
