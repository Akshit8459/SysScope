#pragma once

#include "sysscope/core/timestamp.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

namespace sysscope::core {

struct CoreCpuSnapshot {
    uint32_t core_id{0};
    double total_utilization{0.0};
    double user_percent{0.0};
    double system_percent{0.0};
    double iowait_percent{0.0};
    uint64_t frequency_khz{0};
};

struct CpuSnapshot {
    double total_utilization{0.0};
    double user_percent{0.0};
    double system_percent{0.0};
    double iowait_percent{0.0};
    double irq_percent{0.0};
    double softirq_percent{0.0};
    double steal_percent{0.0};
    std::vector<CoreCpuSnapshot> per_core;
};

struct MemorySnapshot {
    uint64_t total_bytes{0};
    uint64_t available_bytes{0};
    uint64_t free_bytes{0};
    uint64_t used_bytes{0};
    uint64_t cached_bytes{0};
    uint64_t buffers_bytes{0};
    uint64_t swap_total_bytes{0};
    uint64_t swap_free_bytes{0};
    uint64_t swap_used_bytes{0};
    double utilization_percent{0.0};
};

struct ProcessInfo {
    int pid{0};
    int ppid{0};
    std::string name;
    std::string cmdline;
    char state{'R'};
    double cpu_percent{0.0};
    uint64_t rss_bytes{0};
    uint64_t vms_bytes{0};
    uint32_t threads{0};
    uint64_t utime{0};
    uint64_t stime{0};
    uint64_t start_time{0};
};

struct ProcessSnapshot {
    uint32_t total_processes{0};
    uint32_t running_processes{0};
    std::vector<ProcessInfo> process_list;
};

struct GpuSnapshot {
    std::string name;
    double utilization_percent{0.0};
    double memory_utilization_percent{0.0};
    uint64_t memory_used_bytes{0};
    uint64_t memory_total_bytes{0};
    double temperature_celsius{0.0};
    double power_watts{0.0};
};

struct NetworkInterfaceInfo {
    std::string interface_name;
    double rx_bytes_per_sec{0.0};
    double tx_bytes_per_sec{0.0};
    double rx_packets_per_sec{0.0};
    double tx_packets_per_sec{0.0};
    uint64_t rx_errors{0};
    uint64_t tx_errors{0};
    uint64_t rx_drops{0};
    uint64_t tx_drops{0};
};

struct NetworkSnapshot {
    std::vector<NetworkInterfaceInfo> interfaces;
};

struct ThermalZoneInfo {
    std::string name;
    std::string type;
    double temperature_celsius{0.0};
};

struct ThermalSnapshot {
    std::vector<ThermalZoneInfo> zones;
};

struct DiskBlockDeviceInfo {
    std::string device_name;
    double read_iops{0.0};
    double write_iops{0.0};
    double read_bytes_per_sec{0.0};
    double write_bytes_per_sec{0.0};
    double io_utilization_percent{0.0};
    uint32_t queue_depth{0};
};

struct DiskSnapshot {
    std::vector<DiskBlockDeviceInfo> devices;
};

struct PsiPressureValues {
    double avg10{0.0};
    double avg60{0.0};
    double avg300{0.0};
    uint64_t total_microseconds{0};
};

struct PsiSnapshot {
    PsiPressureValues cpu_some;
    PsiPressureValues memory_some;
    PsiPressureValues memory_full;
    PsiPressureValues io_some;
    PsiPressureValues io_full;
};

struct MetricSnapshot {
    Timestamp timestamp;
    CpuSnapshot cpu;
    MemorySnapshot memory;
    ProcessSnapshot processes;
    std::optional<GpuSnapshot> gpu;
    std::optional<NetworkSnapshot> network;
    std::optional<ThermalSnapshot> thermal;
    std::optional<DiskSnapshot> disk;
    std::optional<PsiSnapshot> psi;
};

} // namespace sysscope::core
