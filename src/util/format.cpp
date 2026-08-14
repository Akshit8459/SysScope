#include "sysscope/util/format.hpp"
#include <cstdio>
#include <cmath>
#include <cinttypes>

namespace sysscope::util {

std::string format_metric(double value, int precision) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, value);
    return std::string(buf);
}

std::string format_bytes(uint64_t bytes) {
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double b = static_cast<double>(bytes);
    int unit_idx = 0;
    while (b >= 1024.0 && unit_idx < 4) {
        b /= 1024.0;
        unit_idx++;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f %s", b, units[unit_idx]);
    return std::string(buf);
}

std::string format_bytes_per_sec(double bytes_per_sec) {
    static const char* units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
    double b = bytes_per_sec;
    int unit_idx = 0;
    while (b >= 1024.0 && unit_idx < 3) {
        b /= 1024.0;
        unit_idx++;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.2f %s", b, units[unit_idx]);
    return std::string(buf);
}

std::string format_duration_seconds(double seconds) {
    uint64_t total_sec = static_cast<uint64_t>(seconds);
    uint64_t hours = total_sec / 3600;
    uint64_t minutes = (total_sec % 3600) / 60;
    uint64_t secs = total_sec % 60;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%02" PRIu64 ":%02" PRIu64 ":%02" PRIu64, hours, minutes, secs);
    return std::string(buf);
}

std::string format_frequency_khz(uint64_t khz) {
    if (khz >= 1000000) {
        double ghz = static_cast<double>(khz) / 1000000.0;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.2f GHz", ghz);
        return std::string(buf);
    }
    if (khz >= 1000) {
        double mhz = static_cast<double>(khz) / 1000.0;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.0f MHz", mhz);
        return std::string(buf);
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%" PRIu64 " kHz", khz);
    return std::string(buf);
}

} // namespace sysscope::util
