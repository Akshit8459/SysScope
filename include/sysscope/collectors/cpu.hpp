#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <cstdint>
#include <vector>
#include <memory>

namespace sysscope::collectors {

struct CpuCounters {
    uint64_t user{0};
    uint64_t nice{0};
    uint64_t system{0};
    uint64_t idle{0};
    uint64_t iowait{0};
    uint64_t irq{0};
    uint64_t softirq{0};
    uint64_t steal{0};

    [[nodiscard]] uint64_t total() const noexcept {
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }
    [[nodiscard]] uint64_t idle_time() const noexcept {
        return idle + iowait;
    }
};

class CpuCollector {
public:
    explicit CpuCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~CpuCollector() = default;

    [[nodiscard]] core::Result<core::CpuSnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    CpuCounters previous_aggregate_{};
    std::vector<CpuCounters> previous_cores_{};
    bool has_previous_state_{false};
};

} // namespace sysscope::collectors
