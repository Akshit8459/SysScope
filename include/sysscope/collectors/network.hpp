#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>
#include <unordered_map>

namespace sysscope::collectors {

struct NetworkInterfaceState {
    uint64_t rx_bytes{0};
    uint64_t tx_bytes{0};
    uint64_t rx_packets{0};
    uint64_t tx_packets{0};
    uint64_t rx_errors{0};
    uint64_t tx_errors{0};
    uint64_t rx_drops{0};
    uint64_t tx_drops{0};
    uint64_t timestamp_ns{0};
};

class NetworkCollector {
public:
    explicit NetworkCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~NetworkCollector() = default;

    [[nodiscard]] core::Result<core::NetworkSnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    std::unordered_map<std::string, NetworkInterfaceState> previous_iface_states_;
    bool has_previous_state_{false};
};

} // namespace sysscope::collectors
