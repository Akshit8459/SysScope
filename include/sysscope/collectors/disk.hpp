#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace sysscope::collectors {

struct DiskBlockStats {
    uint64_t reads_completed{0};
    uint64_t read_sectors{0};
    uint64_t read_time_ms{0};
    uint64_t writes_completed{0};
    uint64_t write_sectors{0};
    uint64_t write_time_ms{0};
    uint64_t io_in_progress{0};
    uint64_t io_time_ms{0};
    uint64_t timestamp_ns{0};
};

using DiskInterfaceInfo = core::DiskBlockDeviceInfo;
using DiskSnapshot = core::DiskSnapshot;

class DiskCollector {
public:
    explicit DiskCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~DiskCollector() = default;

    [[nodiscard]] core::Result<core::DiskSnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    std::unordered_map<std::string, DiskBlockStats> previous_disk_states_;
    bool has_previous_state_{false};
};

} // namespace sysscope::collectors
