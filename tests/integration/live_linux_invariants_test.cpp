#include "tests/test_framework.hpp"
#include "sysscope/platform/platform.hpp"
#include "sysscope/platform/real_file_system_reader.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/collectors/process.hpp"
#include "sysscope/collectors/psi.hpp"
#include "sysscope/collectors/network.hpp"
#include "sysscope/collectors/disk.hpp"
#include "sysscope/collectors/thermal.hpp"
#include <memory>
#include <thread>
#include <chrono>

TEST_CASE("LiveLinuxInvariants::KernelTelemetryInvariants") {
    if (!sysscope::platform::is_linux_platform()) {
        // On non-Linux platform hosts, invariant test passes gracefully
        REQUIRE(true);
        return;
    }

    auto fs_reader = std::make_shared<sysscope::platform::RealFileSystemReader>();

    // 1. CPU Collector Invariants
    sysscope::collectors::CpuCollector cpu_col(fs_reader);
    (void)cpu_col.collect(); // Baseline
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto cpu_res = cpu_col.collect();
    REQUIRE(cpu_res.is_ok());
    const auto& cpu_snap = cpu_res.value();
    REQUIRE(cpu_snap.total_utilization >= 0.0 && cpu_snap.total_utilization <= 100.0);
    REQUIRE(cpu_snap.per_core.size() > 0ULL);

    // 2. Memory Collector Invariants
    sysscope::collectors::MemoryCollector mem_col(fs_reader);
    auto mem_res = mem_col.collect();
    REQUIRE(mem_res.is_ok());
    const auto& mem_snap = mem_res.value();
    REQUIRE(mem_snap.total_bytes > 0ULL);
    REQUIRE(mem_snap.available_bytes <= mem_snap.total_bytes);
    REQUIRE(mem_snap.utilization_percent >= 0.0 && mem_snap.utilization_percent <= 100.0);

    // 3. Process Collector Invariants (PID 1 systemd/init must exist)
    sysscope::collectors::ProcessCollector proc_col(fs_reader);
    auto proc_res = proc_col.collect();
    REQUIRE(proc_res.is_ok());
    const auto& proc_snap = proc_res.value();
    REQUIRE(proc_snap.total_processes > 0ULL);
    
    bool found_pid1 = false;
    for (const auto& proc : proc_snap.process_list) {
        if (proc.pid == 1) {
            found_pid1 = true;
            break;
        }
    }
    REQUIRE(found_pid1);

    // 4. PSI Collector Invariants
    sysscope::collectors::PsiCollector psi_col(fs_reader);
    auto psi_res = psi_col.collect();
    REQUIRE(psi_res.is_ok());
    const auto& psi_snap = psi_res.value();
    REQUIRE(psi_snap.cpu_some.avg10 >= 0.0 && psi_snap.cpu_some.avg10 <= 100.0);
    REQUIRE(psi_snap.memory_some.avg10 >= 0.0 && psi_snap.memory_some.avg10 <= 100.0);
    REQUIRE(psi_snap.io_some.avg10 >= 0.0 && psi_snap.io_some.avg10 <= 100.0);

    // 5. Network Collector Invariants
    sysscope::collectors::NetworkCollector net_col(fs_reader);
    (void)net_col.collect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto net_res = net_col.collect();
    REQUIRE(net_res.is_ok());
    REQUIRE(net_res.value().interfaces.size() > 0ULL);
}
