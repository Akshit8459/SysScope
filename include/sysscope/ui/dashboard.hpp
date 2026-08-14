#pragma once

#include "sysscope/core/snapshot.hpp"
#include "sysscope/analytics/correlation.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/collectors/process.hpp"
#include "sysscope/collectors/psi.hpp"
#include "sysscope/collectors/network.hpp"
#include "sysscope/collectors/disk.hpp"
#include "sysscope/collectors/thermal.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>
#include <string>

namespace sysscope::ui {

enum class DashboardScreen {
    Overview = 1,
    CpuCores = 2,
    MemoryAndDisk = 3,
    Processes = 4,
    Diagnostics = 5
};

class Dashboard {
public:
    explicit Dashboard(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~Dashboard() = default;

    void render_snapshot(DashboardScreen screen = DashboardScreen::Overview);
    void run_interactive_loop(int refresh_rate_ms = 500);

private:
    void render_header(const core::MetricSnapshot& snapshot, DashboardScreen active_screen);
    void render_overview(const core::MetricSnapshot& snapshot, const std::vector<analytics::Diagnosis>& diagnoses);
    void render_cpu_cores(const core::MetricSnapshot& snapshot);
    void render_memory_disk(const core::MetricSnapshot& snapshot);
    void render_processes(const core::MetricSnapshot& snapshot);
    void render_diagnostics(const core::MetricSnapshot& snapshot, const std::vector<analytics::Diagnosis>& diagnoses);
    void render_footer(const core::MetricSnapshot& snapshot, DashboardScreen active_screen);

    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    collectors::CpuCollector cpu_col_;
    collectors::MemoryCollector mem_col_;
    collectors::ProcessCollector proc_col_;
    collectors::PsiCollector psi_col_;
    collectors::NetworkCollector net_col_;
    collectors::DiskCollector disk_col_;
    collectors::ThermalCollector thermal_col_;
    analytics::CorrelationEngine correlation_engine_;
    DashboardScreen current_screen_{DashboardScreen::Overview};
    std::vector<std::pair<core::Timestamp, analytics::Diagnosis>> diagnostic_history_;
};

} // namespace sysscope::ui
