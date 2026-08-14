#include "sysscope/core/timestamp.hpp"
#include <ctime>
#include <cstdio>

namespace sysscope::core {

std::string Timestamp::to_iso_string() const {
    std::time_t sec = static_cast<std::time_t>(ns_since_epoch_ / 1000000000ULL);
    uint64_t ms = (ns_since_epoch_ % 1000000000ULL) / 1000000ULL;
    std::tm tm_buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &sec);
#else
    localtime_r(&sec, &tm_buf);
#endif
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                  tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                  tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, static_cast<int>(ms));
    return std::string(buf);
}

} // namespace sysscope::core
