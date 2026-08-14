#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int target_mb = 256;
    int duration_sec = 5;

    if (argc > 1) target_mb = std::atoi(argv[1]);
    if (argc > 2) duration_sec = std::atoi(argv[2]);

    std::cout << "[memory_stressor] Allocating " << target_mb << " MB of RAM for " << duration_sec << " seconds...\n";

    size_t chunk_size = 1024 * 1024; // 1 MB
    std::vector<char*> chunks;

    for (int i = 0; i < target_mb; ++i) {
        char* ptr = new (std::nothrow) char[chunk_size];
        if (ptr) {
            std::memset(ptr, 0xFF, chunk_size); // Dirty the pages to force RSS growth
            chunks.push_back(ptr);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[memory_stressor] Allocation complete. Holding memory for duration...\n";
    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));

    for (char* ptr : chunks) {
        delete[] ptr;
    }

    std::cout << "[memory_stressor] Memory freed.\n";
    return 0;
}
