#include "sysscope/collectors/disk.hpp"
#include <sstream>
#include <algorithm>

namespace sysscope::collectors {

DiskCollector::DiskCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::Result<DiskSnapshot> DiskCollector::collect() {
    auto dev_res = fs_reader_->list_directory_entries("/sys/block");
    if (dev_res.is_err()) {
        return core::Result<DiskSnapshot>::error(dev_res.error_message());
    }

    DiskSnapshot snapshot{};
    uint64_t now_ns = core::Timestamp::now().nanoseconds();
    std::unordered_map<std::string, DiskBlockStats> current_disk_states;

    for (const auto& dev_name : dev_res.value()) {
        if (dev_name.rfind("loop", 0) == 0 || dev_name.rfind("ram", 0) == 0) continue;

        std::string stat_path = "/sys/block/" + dev_name + "/stat";
        auto stat_res = fs_reader_->read_file_contents(stat_path);
        if (stat_res.is_err()) continue;

        std::istringstream ss(stat_res.value());
        DiskBlockStats current_stats{};
        current_stats.timestamp_ns = now_ns;

        uint64_t dummy = 0;
        ss >> current_stats.reads_completed >> dummy >> current_stats.read_sectors >> current_stats.read_time_ms
           >> current_stats.writes_completed >> dummy >> current_stats.write_sectors >> current_stats.write_time_ms
           >> current_stats.io_in_progress >> current_stats.io_time_ms;

        current_disk_states[dev_name] = current_stats;

        DiskInterfaceInfo info{};
        info.device_name = dev_name;
        info.queue_depth = static_cast<uint32_t>(current_stats.io_in_progress);

        if (has_previous_state_) {
            auto prev_it = previous_disk_states_.find(dev_name);
            if (prev_it != previous_disk_states_.end()) {
                uint64_t dt_ns = now_ns - prev_it->second.timestamp_ns;
                if (dt_ns > 0) {
                    double dt_sec = static_cast<double>(dt_ns) / 1e9;
                    uint64_t reads_delta = current_stats.reads_completed - prev_it->second.reads_completed;
                    uint64_t writes_delta = current_stats.writes_completed - prev_it->second.writes_completed;
                    uint64_t r_sectors_delta = current_stats.read_sectors - prev_it->second.read_sectors;
                    uint64_t w_sectors_delta = current_stats.write_sectors - prev_it->second.write_sectors;
                    uint64_t io_ms_delta = current_stats.io_time_ms - prev_it->second.io_time_ms;

                    info.read_iops = static_cast<double>(reads_delta) / dt_sec;
                    info.write_iops = static_cast<double>(writes_delta) / dt_sec;
                    info.read_bytes_per_sec = (static_cast<double>(r_sectors_delta) * 512.0) / dt_sec;
                    info.write_bytes_per_sec = (static_cast<double>(w_sectors_delta) * 512.0) / dt_sec;

                    double io_ms_sec = static_cast<double>(io_ms_delta) / 1000.0;
                    info.io_utilization_percent = std::min(100.0, 100.0 * io_ms_sec / dt_sec);
                }
            }
        }

        snapshot.devices.push_back(info);
    }

    previous_disk_states_ = std::move(current_disk_states);
    has_previous_state_ = true;

    return core::Result<DiskSnapshot>(snapshot);
}

} // namespace sysscope::collectors
