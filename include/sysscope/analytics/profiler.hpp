#pragma once

#include "sysscope/core/snapshot.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/collectors/process.hpp"
#include "sysscope/collectors/psi.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <string>
#include <memory>

namespace sysscope::analytics {

struct ProfileReport {
    std::string command;
    double duration_seconds{0.0};
    
    double avg_cpu_percent{0.0};
    double peak_cpu_percent{0.0};
    double user_cpu_percent{0.0};
    double system_cpu_percent{0.0};
    
    uint64_t initial_rss_bytes{0};
    uint64_t peak_rss_bytes{0};
    uint64_t memory_growth_bytes{0};
    
    double peak_psi_cpu_some{0.0};
    double peak_psi_memory_some{0.0};
    double peak_psi_io_some{0.0};

    std::string summary_diagnosis{"Balanced resource execution."};
};

class ApplicationProfiler {
public:
    explicit ApplicationProfiler(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~ApplicationProfiler() = default;

    [[nodiscard]] ProfileReport profile_command(const std::string& command, int sample_interval_ms = 100);

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
};

} // namespace sysscope::analytics
