#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <cmath>
#include <cstdlib>

void cpu_worker(std::atomic<bool>& running) {
    volatile double dummy = 0.0;
    while (running) {
        for (int i = 0; i < 100000; ++i) {
            dummy += std::sin(i) * std::cos(i);
        }
    }
}

int main(int argc, char* argv[]) {
    int duration_sec = 5;
    int num_threads = 4;

    if (argc > 1) duration_sec = std::atoi(argv[1]);
    if (argc > 2) num_threads = std::atoi(argv[2]);

    std::cout << "[synthetic_load] Spawning " << num_threads << " CPU worker threads for " << duration_sec << " seconds...\n";

    std::atomic<bool> running{true};
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(cpu_worker, std::ref(running));
    }

    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    running = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    std::cout << "[synthetic_load] CPU stress test completed.\n";
    return 0;
}
