#include "tests/test_framework.hpp"
#include "sysscope/collectors/psi.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include <memory>

TEST_CASE("Collectors::PsiCollector Pressure Stall Information Parsing") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    std::string cpu_psi = "some avg10=8.20 avg60=4.10 avg300=2.05 total=1254000\n";
    std::string mem_psi = 
        "some avg10=14.50 avg60=9.30 avg300=5.10 total=3400000\n"
        "full avg10=2.10 avg60=1.05 avg300=0.40 total=510000\n";
    std::string io_psi = 
        "some avg10=3.20 avg60=1.80 avg300=0.90 total=890000\n"
        "full avg10=0.50 avg60=0.20 avg300=0.10 total=120000\n";

    mock_fs->set_file_content("/proc/pressure/cpu", cpu_psi);
    mock_fs->set_file_content("/proc/pressure/memory", mem_psi);
    mock_fs->set_file_content("/proc/pressure/io", io_psi);

    sysscope::collectors::PsiCollector collector(mock_fs);
    auto res = collector.collect();

    REQUIRE(res.is_ok());
    const auto& snap = res.value();

    REQUIRE(snap.cpu_some.avg10 >= 8.19 && snap.cpu_some.avg10 <= 8.21);
    REQUIRE(snap.memory_some.avg10 >= 14.49 && snap.memory_some.avg10 <= 14.51);
    REQUIRE(snap.memory_full.avg10 >= 2.09 && snap.memory_full.avg10 <= 2.11);
    REQUIRE(snap.io_some.avg10 >= 3.19 && snap.io_some.avg10 <= 3.21);
}
