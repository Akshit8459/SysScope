#pragma once

#include <string>

namespace sysscope::platform {

[[nodiscard]] inline constexpr bool is_linux_platform() noexcept {
#if defined(__linux__)
    return true;
#else
    return false;
#endif
}

[[nodiscard]] inline std::string get_platform_name() {
#if defined(__linux__)
    return "Linux";
#elif defined(_WIN32) || defined(_WIN64)
    return "Windows (Host)";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Unknown POSIX Target";
#endif
}

} // namespace sysscope::platform
