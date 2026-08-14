#include "tests/test_framework.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include <memory>

TEST_CASE("Collectors::CpuCollector Single and Two Snapshot Utilization Calculations") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    // Snapshot T0
    std::string stat_t0 = 
        "cpu  1000 0 1000 8000 0 0 0 0\n"
        "cpu0 500 0 500 4000 0 0 0 0\n"
        "cpu1 500 0 500 4000 0 0 0 0\n";

    mock_fs->set_file_content("/proc/stat", stat_t0);
    mock_fs->set_file_content("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "2400000");
    mock_fs->set_file_content("/sys/devices/system/cpu/cpu1/cpufreq/scaling_cur_freq", "2400000");

    sysscope::collectors::CpuCollector collector(mock_fs);

    // Initial snapshot establishes baseline (utilization is 0 on first call)
    auto res_t0 = collector.collect();
    REQUIRE(res_t0.is_ok());
    REQUIRE_EQ(res_t0.value().per_core.size(), 2ULL);
    REQUIRE_EQ(res_t0.value().per_core[0].frequency_khz, 2400000ULL);

    // Snapshot T1: 1000 additional total ticks, 500 active, 500 idle -> 50% utilization
    std::string stat_t1 = 
        "cpu  1250 0 1250 8500 0 0 0 0\n"
        "cpu0 625 0 625 4250 0 0 0 0\n"
        "cpu1 625 0 625 4250 0 0 0 0\n";

    mock_fs->set_file_content("/proc/stat", stat_t1);

    auto res_t1 = collector.collect();
    REQUIRE(res_t1.is_ok());
    
    double util = res_t1.value().total_utilization;
    REQUIRE(util >= 49.9 && util <= 50.1);
    REQUIRE(res_t1.value().user_percent >= 24.9 && res_t1.value().user_percent <= 25.1);
    REQUIRE(res_t1.value().system_percent >= 24.9 && res_t1.value().system_percent <= 25.1);
}
