#include "tests/test_framework.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include <memory>

TEST_CASE("Collectors::MemoryCollector MemAvailable Priority Calculation") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    std::string meminfo_content =
        "MemTotal:       16000000 kB\n"
        "MemFree:         4000000 kB\n"
        "MemAvailable:   12000000 kB\n"
        "Buffers:          500000 kB\n"
        "Cached:          3500000 kB\n"
        "SwapTotal:       4000000 kB\n"
        "SwapFree:        3000000 kB\n";

    mock_fs->set_file_content("/proc/meminfo", meminfo_content);

    sysscope::collectors::MemoryCollector collector(mock_fs);
    auto res = collector.collect();

    REQUIRE(res.is_ok());
    const auto& snap = res.value();

    REQUIRE_EQ(snap.total_bytes, 16000000ULL * 1024ULL);
    REQUIRE_EQ(snap.available_bytes, 12000000ULL * 1024ULL);
    REQUIRE_EQ(snap.used_bytes, 4000000ULL * 1024ULL);
    
    // Utilization should be 4GB / 16GB = 25%
    REQUIRE(snap.utilization_percent >= 24.9 && snap.utilization_percent <= 25.1);
    REQUIRE_EQ(snap.swap_used_bytes, 1000000ULL * 1024ULL);
}
