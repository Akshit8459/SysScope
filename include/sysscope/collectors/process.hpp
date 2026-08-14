#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace sysscope::collectors {

struct ProcessNode {
    core::ProcessInfo info;
    std::vector<int> children_pids;
};

struct ProcessStateSnapshot {
    uint64_t total_process_cpu_time{0};
    uint64_t timestamp_ns{0};
};

class ProcessCollector {
public:
    explicit ProcessCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~ProcessCollector() = default;

    [[nodiscard]] core::Result<core::ProcessSnapshot> collect();
    [[nodiscard]] std::unordered_map<int, ProcessNode> build_process_tree(const core::ProcessSnapshot& snapshot) const;

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    std::unordered_map<int, ProcessStateSnapshot> previous_process_states_;
    uint64_t previous_total_system_time_{0};
    bool has_previous_state_{false};
    long ticks_per_sec_{100};
};

} // namespace sysscope::collectors
