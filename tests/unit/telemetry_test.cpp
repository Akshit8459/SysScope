#include "tests/test_framework.hpp"
#include "sysscope/telemetry/queue.hpp"
#include "sysscope/telemetry/scheduler.hpp"
#include <atomic>
#include <thread>

TEST_CASE("Telemetry::BoundedQueue Capacity Overflow and Push/Pop") {
    sysscope::telemetry::BoundedQueue<int> queue(3);

    queue.push(1);
    queue.push(2);
    queue.push(3);
    REQUIRE_EQ(queue.size(), 3ULL);
    REQUIRE_EQ(queue.dropped_count(), 0ULL);

    // Overflow pushes 4th item, dropping 1st (item 1)
    queue.push(4);
    REQUIRE_EQ(queue.size(), 3ULL);
    REQUIRE_EQ(queue.dropped_count(), 1ULL);

    auto item = queue.pop();
    REQUIRE(item.has_value());
    REQUIRE_EQ(item.value(), 2); // 1 was dropped, so 2 is first
}

TEST_CASE("Telemetry::TelemetryScheduler Multi-Rate Task Execution") {
    sysscope::telemetry::TelemetryScheduler scheduler;
    std::atomic<int> fast_count{0};
    std::atomic<int> slow_count{0};

    scheduler.add_task("fast_task", std::chrono::milliseconds(10), [&] {
        fast_count++;
    });

    scheduler.add_task("slow_task", std::chrono::milliseconds(50), [&] {
        slow_count++;
    });

    scheduler.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    scheduler.stop();

    REQUIRE(fast_count > 5);
    REQUIRE(slow_count >= 2);
}
