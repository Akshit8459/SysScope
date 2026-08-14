#pragma once

#include <cstdint>
#include <string_view>

namespace sysscope::core {

enum class MetricType : uint8_t {
    CpuUtilization,
    CpuUserPercent,
    CpuSystemPercent,
    CpuIoWaitPercent,
    CpuFrequency,
    
    MemoryUtilization,
    MemoryTotal,
    MemoryAvailable,
    MemoryFree,
    MemoryCached,
    MemoryBuffers,
    SwapUtilization,
    
    ProcessCpu,
    ProcessRssMemory,
    ProcessVmsMemory,
    ProcessThreads,
    
    GpuUtilization,
    GpuMemoryUtilization,
    GpuMemoryUsed,
    GpuTemperature,
    GpuPower,
    
    NetworkRxRate,
    NetworkTxRate,
    NetworkRxErrors,
    NetworkTxErrors,
    
    DiskReadIops,
    DiskWriteIops,
    DiskReadThroughput,
    DiskWriteThroughput,
    DiskIoUtilization,
    
    Temperature,
    PowerConsumption,
    
    PsiCpuSome,
    PsiMemorySome,
    PsiMemoryFull,
    PsiIoSome,
    PsiIoFull
};

enum class Unit : uint8_t {
    Percent,
    Bytes,
    Kilobytes,
    Megabytes,
    Gigabytes,
    BytesPerSec,
    MegabytesPerSec,
    PacketsPerSec,
    Iops,
    Celsius,
    Watts,
    Hertz,
    Kilohertz,
    Megahertz,
    Count,
    Microseconds
};

enum class Source : uint8_t {
    Procfs,
    Sysfs,
    Netlink,
    NVML,
    Qualcomm,
    Hwmon,
    Mock
};

[[nodiscard]] constexpr std::string_view to_string(MetricType type) noexcept {
    switch (type) {
        case MetricType::CpuUtilization: return "CpuUtilization";
        case MetricType::CpuUserPercent: return "CpuUserPercent";
        case MetricType::CpuSystemPercent: return "CpuSystemPercent";
        case MetricType::CpuIoWaitPercent: return "CpuIoWaitPercent";
        case MetricType::CpuFrequency: return "CpuFrequency";
        case MetricType::MemoryUtilization: return "MemoryUtilization";
        case MetricType::MemoryTotal: return "MemoryTotal";
        case MetricType::MemoryAvailable: return "MemoryAvailable";
        case MetricType::MemoryFree: return "MemoryFree";
        case MetricType::MemoryCached: return "MemoryCached";
        case MetricType::MemoryBuffers: return "MemoryBuffers";
        case MetricType::SwapUtilization: return "SwapUtilization";
        case MetricType::ProcessCpu: return "ProcessCpu";
        case MetricType::ProcessRssMemory: return "ProcessRssMemory";
        case MetricType::ProcessVmsMemory: return "ProcessVmsMemory";
        case MetricType::ProcessThreads: return "ProcessThreads";
        case MetricType::GpuUtilization: return "GpuUtilization";
        case MetricType::GpuMemoryUtilization: return "GpuMemoryUtilization";
        case MetricType::GpuMemoryUsed: return "GpuMemoryUsed";
        case MetricType::GpuTemperature: return "GpuTemperature";
        case MetricType::GpuPower: return "GpuPower";
        case MetricType::NetworkRxRate: return "NetworkRxRate";
        case MetricType::NetworkTxRate: return "NetworkTxRate";
        case MetricType::NetworkRxErrors: return "NetworkRxErrors";
        case MetricType::NetworkTxErrors: return "NetworkTxErrors";
        case MetricType::DiskReadIops: return "DiskReadIops";
        case MetricType::DiskWriteIops: return "DiskWriteIops";
        case MetricType::DiskReadThroughput: return "DiskReadThroughput";
        case MetricType::DiskWriteThroughput: return "DiskWriteThroughput";
        case MetricType::DiskIoUtilization: return "DiskIoUtilization";
        case MetricType::Temperature: return "Temperature";
        case MetricType::PowerConsumption: return "PowerConsumption";
        case MetricType::PsiCpuSome: return "PsiCpuSome";
        case MetricType::PsiMemorySome: return "PsiMemorySome";
        case MetricType::PsiMemoryFull: return "PsiMemoryFull";
        case MetricType::PsiIoSome: return "PsiIoSome";
        case MetricType::PsiIoFull: return "PsiIoFull";
    }
    return "Unknown";
}

[[nodiscard]] constexpr std::string_view to_string(Unit unit) noexcept {
    switch (unit) {
        case Unit::Percent: return "%";
        case Unit::Bytes: return "B";
        case Unit::Kilobytes: return "KB";
        case Unit::Megabytes: return "MB";
        case Unit::Gigabytes: return "GB";
        case Unit::BytesPerSec: return "B/s";
        case Unit::MegabytesPerSec: return "MB/s";
        case Unit::PacketsPerSec: return "pkt/s";
        case Unit::Iops: return "IOPS";
        case Unit::Celsius: return "°C";
        case Unit::Watts: return "W";
        case Unit::Hertz: return "Hz";
        case Unit::Kilohertz: return "kHz";
        case Unit::Megahertz: return "MHz";
        case Unit::Count: return "cnt";
        case Unit::Microseconds: return "us";
    }
    return "";
}

[[nodiscard]] constexpr std::string_view to_string(Source source) noexcept {
    switch (source) {
        case Source::Procfs: return "procfs";
        case Source::Sysfs: return "sysfs";
        case Source::Netlink: return "netlink";
        case Source::NVML: return "nvml";
        case Source::Qualcomm: return "qualcomm";
        case Source::Hwmon: return "hwmon";
        case Source::Mock: return "mock";
    }
    return "unknown";
}

} // namespace sysscope::core
