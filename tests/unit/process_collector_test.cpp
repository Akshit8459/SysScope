#include "tests/test_framework.hpp"
#include "sysscope/collectors/process.hpp"
#include "sysscope/platform/mock_file_system_reader.hpp"
#include <memory>

TEST_CASE("Collectors::ProcessCollector Process Parsing and Process Tree Building") {
    auto mock_fs = std::make_shared<sysscope::platform::MockFileSystemReader>();

    mock_fs->add_directory_entry("/proc", "1");
    mock_fs->add_directory_entry("/proc", "1821");
    mock_fs->add_directory_entry("/proc", "1822");

    // /proc/stat snapshot T0
    mock_fs->set_file_content("/proc/stat", "cpu  1000 0 1000 8000 0 0 0 0\n");

    // PID 1: systemd (PPID 0)
    std::string pid1_stat = "1 (systemd) S 0 1 1 0 -1 4194560 100 0 0 0 10 20 0 0 20 0 1 0 1000 200000 500 18446744073709551615\n";
    mock_fs->set_file_content("/proc/1/stat", pid1_stat);
    mock_fs->set_file_content("/proc/1/cmdline", "/sbin/systemd");

    // PID 1821: inference (PPID 1)
    std::string pid1821_stat = "1821 (inference) R 1 1821 1 0 -1 4194560 100 0 0 0 100 200 0 0 20 0 8 0 1000 800000 1000 18446744073709551615\n";
    mock_fs->set_file_content("/proc/1821/stat", pid1821_stat);
    mock_fs->set_file_content("/proc/1821/cmdline", "./inference-engine");

    // PID 1822: worker-1 (PPID 1821)
    std::string pid1822_stat = "1822 (worker-1) S 1821 1821 1 0 -1 4194560 100 0 0 0 50 50 0 0 20 0 4 0 1000 400000 500 18446744073709551615\n";
    mock_fs->set_file_content("/proc/1822/stat", pid1822_stat);
    mock_fs->set_file_content("/proc/1822/cmdline", "./worker-1");

    sysscope::collectors::ProcessCollector collector(mock_fs);

    // Initial collection establishes PID state baseline
    auto res_t0 = collector.collect();
    REQUIRE(res_t0.is_ok());
    REQUIRE_EQ(res_t0.value().process_list.size(), 3ULL);

    // Build process tree
    auto tree = collector.build_process_tree(res_t0.value());
    REQUIRE_EQ(tree.size(), 3ULL);

    // PID 1821 should have child PID 1822
    REQUIRE_EQ(tree[1821].children_pids.size(), 1ULL);
    REQUIRE_EQ(tree[1821].children_pids[0], 1822);
}
