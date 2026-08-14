#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace sysscope::core {

class Timestamp {
public:
    Timestamp() noexcept
        : ns_since_epoch_(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count())) {}

    explicit Timestamp(uint64_t nanoseconds) noexcept
        : ns_since_epoch_(nanoseconds) {}

    [[nodiscard]] uint64_t nanoseconds() const noexcept { return ns_since_epoch_; }
    [[nodiscard]] double seconds() const noexcept { return static_cast<double>(ns_since_epoch_) / 1e9; }

    [[nodiscard]] std::string to_iso_string() const;

    static Timestamp now() noexcept { return Timestamp{}; }

    bool operator==(const Timestamp& other) const noexcept { return ns_since_epoch_ == other.ns_since_epoch_; }
    bool operator!=(const Timestamp& other) const noexcept { return ns_since_epoch_ != other.ns_since_epoch_; }
    bool operator<(const Timestamp& other) const noexcept { return ns_since_epoch_ < other.ns_since_epoch_; }
    bool operator<=(const Timestamp& other) const noexcept { return ns_since_epoch_ <= other.ns_since_epoch_; }
    bool operator>(const Timestamp& other) const noexcept { return ns_since_epoch_ > other.ns_since_epoch_; }
    bool operator>=(const Timestamp& other) const noexcept { return ns_since_epoch_ >= other.ns_since_epoch_; }

private:
    uint64_t ns_since_epoch_{0};
};

} // namespace sysscope::core
