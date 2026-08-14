#pragma once

#include <cstdint>
#include <string>

namespace sysscope::util {

std::string format_metric(double value, int precision = 2);
std::string format_bytes(uint64_t bytes);
std::string format_bytes_per_sec(double bytes_per_sec);
std::string format_duration_seconds(double seconds);
std::string format_frequency_khz(uint64_t khz);

} // namespace sysscope::util
