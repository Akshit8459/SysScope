#include "sysscope/collectors/cpu.hpp"
#include <sstream>
#include <algorithm>

namespace sysscope::collectors {

CpuCollector::CpuCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::Result<core::CpuSnapshot> CpuCollector::collect() {
    auto lines_res = fs_reader_->read_file_lines("/proc/stat");
    if (lines_res.is_err()) {
        return core::Result<core::CpuSnapshot>::error(lines_res.error_message());
    }

    const auto& lines = lines_res.value();
    CpuCounters current_aggregate{};
    std::vector<CpuCounters> current_cores;

    for (const auto& line : lines) {
        if (line.rfind("cpu", 0) != 0) {
            continue; // Not a CPU line
        }

        std::istringstream ss(line);
        std::string label;
        CpuCounters counters{};

        ss >> label >> counters.user >> counters.nice >> counters.system
           >> counters.idle >> counters.iowait >> counters.irq
           >> counters.softirq >> counters.steal;

        if (label == "cpu") {
            current_aggregate = counters;
        } else if (label.find("cpu") == 0) {
            current_cores.push_back(counters);
        }
    }

    core::CpuSnapshot snapshot{};

    if (!has_previous_state_) {
        previous_aggregate_ = current_aggregate;
        previous_cores_ = current_cores;
        has_previous_state_ = true;

        // Populate initial core structures with frequency
        for (size_t i = 0; i < current_cores.size(); ++i) {
            core::CoreCpuSnapshot core_snap{};
            core_snap.core_id = static_cast<uint32_t>(i);
            
            // Attempt to read scaling_cur_freq
            std::string freq_path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_cur_freq";
            auto freq_res = fs_reader_->read_file_contents(freq_path);
            if (freq_res.is_ok()) {
                try {
                    core_snap.frequency_khz = std::stoull(freq_res.value());
                } catch (...) {}
            }
            snapshot.per_core.push_back(core_snap);
        }

        return core::Result<core::CpuSnapshot>(snapshot);
    }

    // Compute aggregate delta
    uint64_t total_delta = current_aggregate.total() - previous_aggregate_.total();
    uint64_t idle_delta = current_aggregate.idle_time() - previous_aggregate_.idle_time();
    uint64_t user_delta = current_aggregate.user - previous_aggregate_.user;
    uint64_t system_delta = current_aggregate.system - previous_aggregate_.system;
    uint64_t iowait_delta = current_aggregate.iowait - previous_aggregate_.iowait;
    uint64_t irq_delta = current_aggregate.irq - previous_aggregate_.irq;
    uint64_t softirq_delta = current_aggregate.softirq - previous_aggregate_.softirq;
    uint64_t steal_delta = current_aggregate.steal - previous_aggregate_.steal;

    if (total_delta > 0) {
        snapshot.total_utilization = std::min(100.0, 100.0 * static_cast<double>(total_delta - idle_delta) / static_cast<double>(total_delta));
        snapshot.user_percent = 100.0 * static_cast<double>(user_delta) / static_cast<double>(total_delta);
        snapshot.system_percent = 100.0 * static_cast<double>(system_delta) / static_cast<double>(total_delta);
        snapshot.iowait_percent = 100.0 * static_cast<double>(iowait_delta) / static_cast<double>(total_delta);
        snapshot.irq_percent = 100.0 * static_cast<double>(irq_delta) / static_cast<double>(total_delta);
        snapshot.softirq_percent = 100.0 * static_cast<double>(softirq_delta) / static_cast<double>(total_delta);
        snapshot.steal_percent = 100.0 * static_cast<double>(steal_delta) / static_cast<double>(total_delta);
    }

    // Compute per-core deltas
    for (size_t i = 0; i < current_cores.size(); ++i) {
        core::CoreCpuSnapshot core_snap{};
        core_snap.core_id = static_cast<uint32_t>(i);

        if (i < previous_cores_.size()) {
            uint64_t c_total_delta = current_cores[i].total() - previous_cores_[i].total();
            uint64_t c_idle_delta = current_cores[i].idle_time() - previous_cores_[i].idle_time();
            uint64_t c_user_delta = current_cores[i].user - previous_cores_[i].user;
            uint64_t c_sys_delta = current_cores[i].system - previous_cores_[i].system;
            uint64_t c_io_delta = current_cores[i].iowait - previous_cores_[i].iowait;

            if (c_total_delta > 0) {
                core_snap.total_utilization = std::min(100.0, 100.0 * static_cast<double>(c_total_delta - c_idle_delta) / static_cast<double>(c_total_delta));
                core_snap.user_percent = 100.0 * static_cast<double>(c_user_delta) / static_cast<double>(c_total_delta);
                core_snap.system_percent = 100.0 * static_cast<double>(c_sys_delta) / static_cast<double>(c_total_delta);
                core_snap.iowait_percent = 100.0 * static_cast<double>(c_io_delta) / static_cast<double>(c_total_delta);
            }
        }

        std::string freq_path = "/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_cur_freq";
        auto freq_res = fs_reader_->read_file_contents(freq_path);
        if (freq_res.is_ok()) {
            try {
                core_snap.frequency_khz = std::stoull(freq_res.value());
            } catch (...) {}
        }

        snapshot.per_core.push_back(core_snap);
    }

    // Update baseline
    previous_aggregate_ = current_aggregate;
    previous_cores_ = current_cores;

    return core::Result<core::CpuSnapshot>(snapshot);
}

} // namespace sysscope::collectors
