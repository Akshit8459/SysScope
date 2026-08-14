#include "tests/test_framework.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/collectors/process.hpp"
#include "sysscope/collectors/psi.hpp"
#include "sysscope/collectors/network.hpp"
#include "sysscope/collectors/disk.hpp"
#include "sysscope/collectors/thermal.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include "sysscope/platform/netlink_reader.hpp"
#include "sysscope/platform/real_file_system_reader.hpp"
#include <memory>

TEST_CASE("FailurePaths::MissingProcStat Handling") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();
    sysscope::collectors::CpuCollector collector(mock_fs);

    auto res = collector.collect();
    REQUIRE(res.is_err());
    REQUIRE(!res.error_message().empty());
}

TEST_CASE("FailurePaths::MalformedMeminfo Handling") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();
    mock_fs->set_file_content("/proc/meminfo", "CORRUPT_HEADER_WITHOUT_COLON\nINVALID_LINE_123\n");

    sysscope::collectors::MemoryCollector collector(mock_fs);
    auto res = collector.collect();
    REQUIRE(res.is_ok()); // Should return 0 values without crashing
    REQUIRE_EQ(res.value().total_bytes, 0ULL);
}

TEST_CASE("FailurePaths::MalformedProcessStat Handling") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();
    mock_fs->add_directory_entry("/proc", "999");
    // Missing matching parentheses in stat
    mock_fs->set_file_content("/proc/999/stat", "999 malformed_name_no_parens R 1 2 3\n");

    sysscope::collectors::ProcessCollector collector(mock_fs);
    auto res = collector.collect();
    REQUIRE(res.is_ok());
    // Corrupt process line should be skipped cleanly without crashing
    REQUIRE_EQ(res.value().process_list.size(), 0ULL);
}

TEST_CASE("FailurePaths::MissingPsiFiles Handling") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();
    sysscope::collectors::PsiCollector collector(mock_fs);

    auto res = collector.collect();
    REQUIRE(res.is_ok());
    REQUIRE_EQ(res.value().cpu_some.avg10, 0.0);
}

TEST_CASE("Platform::MockNetlinkReader Link Stats Query") {
    sysscope::platform::MockNetlinkReader netlink(true);
    sysscope::platform::NetlinkLinkStats link1{"eth0", 1048576, 524288, 1000, 500, 0, 0, 0, 0};
    netlink.add_mock_link(link1);

    REQUIRE(netlink.is_available());
    auto res = netlink.query_link_stats();
    REQUIRE(res.is_ok());
    REQUIRE_EQ(res.value().size(), 1ULL);
    REQUIRE_EQ(res.value()[0].interface_name, "eth0");
}

TEST_CASE("Platform::RealFileSystemReader Error On NonExistent File") {
    sysscope::platform::RealFileSystemReader real_fs;
    auto res = real_fs.read_file_contents("/non_existent_sysscope_file.txt");
    REQUIRE(res.is_err());
}
