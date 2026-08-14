#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace sysscope::telemetry {

struct SamplingTask {
    std::string name;
    std::chrono::milliseconds interval;
    std::chrono::steady_clock::time_point next_deadline;
    std::function<void()> callback;
};

class TelemetryScheduler {
public:
    TelemetryScheduler();
    ~TelemetryScheduler();

    void add_task(std::string name, std::chrono::milliseconds interval, std::function<void()> callback);
    void start();
    void stop();

    [[nodiscard]] bool is_running() const noexcept { return running_; }

private:
    void run_loop();

    std::vector<SamplingTask> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread worker_thread_;
    std::atomic<bool> running_{false};
};

} // namespace sysscope::telemetry
