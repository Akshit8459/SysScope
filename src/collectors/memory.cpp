#include "sysscope/collectors/memory.hpp"
#include <sstream>
#include <unordered_map>
#include <algorithm>

namespace sysscope::collectors {

MemoryCollector::MemoryCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::Result<core::MemorySnapshot> MemoryCollector::collect() {
    auto lines_res = fs_reader_->read_file_lines("/proc/meminfo");
    if (lines_res.is_err()) {
        return core::Result<core::MemorySnapshot>::error(lines_res.error_message());
    }

    std::unordered_map<std::string, uint64_t> mem_fields;
    for (const auto& line : lines_res.value()) {
        std::istringstream ss(line);
        std::string key;
        uint64_t val_kb = 0;
        if (ss >> key >> val_kb) {
            if (!key.empty() && key.back() == ':') {
                key.pop_back();
            }
            mem_fields[key] = val_kb * 1024ULL; // Convert kB to Bytes
        }
    }

    core::MemorySnapshot snapshot{};
    snapshot.total_bytes = mem_fields["MemTotal"];
    snapshot.free_bytes = mem_fields["MemFree"];
    snapshot.available_bytes = mem_fields["MemAvailable"];
    snapshot.buffers_bytes = mem_fields["Buffers"];
    snapshot.cached_bytes = mem_fields["Cached"];
    snapshot.swap_total_bytes = mem_fields["SwapTotal"];
    snapshot.swap_free_bytes = mem_fields["SwapFree"];

    // Fallback if MemAvailable is missing in older kernels
    if (snapshot.available_bytes == 0 && snapshot.total_bytes > 0) {
        snapshot.available_bytes = snapshot.free_bytes + snapshot.buffers_bytes + snapshot.cached_bytes;
    }

    if (snapshot.total_bytes > snapshot.available_bytes) {
        snapshot.used_bytes = snapshot.total_bytes - snapshot.available_bytes;
    } else {
        snapshot.used_bytes = 0;
    }

    if (snapshot.total_bytes > 0) {
        snapshot.utilization_percent = std::min(100.0, 100.0 * static_cast<double>(snapshot.used_bytes) / static_cast<double>(snapshot.total_bytes));
    }

    if (snapshot.swap_total_bytes > snapshot.swap_free_bytes) {
        snapshot.swap_used_bytes = snapshot.swap_total_bytes - snapshot.swap_free_bytes;
    } else {
        snapshot.swap_used_bytes = 0;
    }

    return core::Result<core::MemorySnapshot>(snapshot);
}

} // namespace sysscope::collectors
