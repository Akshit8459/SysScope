#include "tests/test_framework.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"

TEST_CASE("Platform::MockFileSystemReader File and Directory Operations") {
    sysscope::platform::MockFileSystemReader mock_fs;
    mock_fs.set_file_content("/proc/stat", "cpu  100 20 50 800 10 5 2 0 0 0\n");
    mock_fs.add_directory_entry("/proc", "stat");
    mock_fs.add_directory_entry("/proc", "meminfo");

    REQUIRE(mock_fs.file_exists("/proc/stat"));
    REQUIRE(mock_fs.file_exists("/proc"));

    auto content_res = mock_fs.read_file_contents("/proc/stat");
    REQUIRE(content_res.is_ok());
    REQUIRE_EQ(content_res.value(), "cpu  100 20 50 800 10 5 2 0 0 0\n");

    auto lines_res = mock_fs.read_file_lines("/proc/stat");
    REQUIRE(lines_res.is_ok());
    REQUIRE_EQ(lines_res.value().size(), 1ULL);

    auto dir_res = mock_fs.list_directory_entries("/proc");
    REQUIRE(dir_res.is_ok());
    REQUIRE_EQ(dir_res.value().size(), 2ULL);

    auto missing_res = mock_fs.read_file_contents("/proc/nonexistent");
    REQUIRE(missing_res.is_err());
}
