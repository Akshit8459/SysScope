#include "tests/test_framework.hpp"
#include "sysscope/core/timestamp.hpp"
#include "sysscope/core/types.hpp"
#include "sysscope/util/format.hpp"

TEST_CASE("Core::Timestamp Basic Functionality") {
    sysscope::core::Timestamp ts1(1000000000ULL); // 1 sec
    REQUIRE_EQ(ts1.nanoseconds(), 1000000000ULL);
    REQUIRE(ts1.seconds() > 0.99 && ts1.seconds() < 1.01);

    sysscope::core::Timestamp ts_now = sysscope::core::Timestamp::now();
    REQUIRE(ts_now.nanoseconds() > 0);
}

TEST_CASE("Core::Types String Conversions") {
    REQUIRE_EQ(sysscope::core::to_string(sysscope::core::MetricType::CpuUtilization), "CpuUtilization");
    REQUIRE_EQ(sysscope::core::to_string(sysscope::core::MetricType::MemoryAvailable), "MemoryAvailable");
    REQUIRE_EQ(sysscope::core::to_string(sysscope::core::Unit::Megabytes), "MB");
    REQUIRE_EQ(sysscope::core::to_string(sysscope::core::Source::Procfs), "procfs");
}

TEST_CASE("Util::Format Formatting Helpers") {
    REQUIRE_EQ(sysscope::util::format_metric(82.456, 1), "82.5");
    REQUIRE_EQ(sysscope::util::format_bytes(1024), "1.00 KB");
    REQUIRE_EQ(sysscope::util::format_bytes(1048576), "1.00 MB");
    REQUIRE_EQ(sysscope::util::format_frequency_khz(3600000), "3.60 GHz");
}
