#include "sysscope/collectors/network.hpp"
#include <sstream>

namespace sysscope::collectors {

NetworkCollector::NetworkCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::Result<core::NetworkSnapshot> NetworkCollector::collect() {
    auto lines_res = fs_reader_->read_file_lines("/proc/net/dev");
    if (lines_res.is_err()) {
        return core::Result<core::NetworkSnapshot>::error(lines_res.error_message());
    }

    core::NetworkSnapshot snapshot{};
    uint64_t now_ns = core::Timestamp::now().nanoseconds();
    std::unordered_map<std::string, NetworkInterfaceState> current_iface_states;

    for (const auto& line : lines_res.value()) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;

        std::string iface_name = line.substr(0, colon_pos);
        // Trim leading spaces
        iface_name.erase(0, iface_name.find_first_not_of(" \t"));
        if (iface_name.empty() || iface_name == "lo") continue;

        std::string rest = line.substr(colon_pos + 1);
        std::istringstream ss(rest);

        NetworkInterfaceState current_state{};
        current_state.timestamp_ns = now_ns;

        uint64_t dummy = 0;
        ss >> current_state.rx_bytes >> current_state.rx_packets >> current_state.rx_errors
           >> current_state.rx_drops >> dummy >> dummy >> dummy >> dummy
           >> current_state.tx_bytes >> current_state.tx_packets >> current_state.tx_errors
           >> current_state.tx_drops;

        current_iface_states[iface_name] = current_state;

        core::NetworkInterfaceInfo info{};
        info.interface_name = iface_name;
        info.rx_errors = current_state.rx_errors;
        info.tx_errors = current_state.tx_errors;
        info.rx_drops = current_state.rx_drops;
        info.tx_drops = current_state.tx_drops;

        if (has_previous_state_) {
            auto prev_it = previous_iface_states_.find(iface_name);
            if (prev_it != previous_iface_states_.end()) {
                uint64_t dt_ns = now_ns - prev_it->second.timestamp_ns;
                if (dt_ns > 0) {
                    double dt_sec = static_cast<double>(dt_ns) / 1e9;
                    uint64_t rx_bytes_delta = current_state.rx_bytes - prev_it->second.rx_bytes;
                    uint64_t tx_bytes_delta = current_state.tx_bytes - prev_it->second.tx_bytes;
                    uint64_t rx_pkt_delta = current_state.rx_packets - prev_it->second.rx_packets;
                    uint64_t tx_pkt_delta = current_state.tx_packets - prev_it->second.tx_packets;

                    info.rx_bytes_per_sec = static_cast<double>(rx_bytes_delta) / dt_sec;
                    info.tx_bytes_per_sec = static_cast<double>(tx_bytes_delta) / dt_sec;
                    info.rx_packets_per_sec = static_cast<double>(rx_pkt_delta) / dt_sec;
                    info.tx_packets_per_sec = static_cast<double>(tx_pkt_delta) / dt_sec;
                }
            }
        }

        snapshot.interfaces.push_back(info);
    }

    previous_iface_states_ = std::move(current_iface_states);
    has_previous_state_ = true;

    return core::Result<core::NetworkSnapshot>(snapshot);
}

} // namespace sysscope::collectors
