#include "sysscope/core/version.hpp"
#include "sysscope/ui/dashboard.hpp"
#include "sysscope/ui/terminal.hpp"
#include "sysscope/util/format.hpp"
#include "sysscope/platform/platform.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <chrono>

namespace sysscope::ui {

namespace colors {
    static const char* RESET   = "\033[0m";
    static const char* BOLD    = "\033[1m";
    static const char* RED     = "\033[31m";
    static const char* GREEN   = "\033[32m";
    static const char* YELLOW  = "\033[33m";
    static const char* CYAN    = "\033[36m";
    static const char* WHITE   = "\033[37m";
    static const char* BG_BLUE = "\033[44m";
}

Dashboard::Dashboard(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(fs_reader),
      cpu_col_(fs_reader),
      mem_col_(fs_reader),
      proc_col_(fs_reader),
      psi_col_(fs_reader),
      net_col_(fs_reader),
      disk_col_(fs_reader),
      thermal_col_(fs_reader) {}

static std::string make_progress_bar(double percent, int width = 20) {
    percent = std::clamp(percent, 0.0, 100.0);
    int filled = static_cast<int>((percent / 100.0) * static_cast<double>(width));
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) bar += "█";
        else bar += "░";
    }
    bar += "]";
    return bar;
}

void Dashboard::render_header(const core::MetricSnapshot& snapshot, DashboardScreen active_screen) {
    Terminal::clear_screen();
    std::cout << colors::BG_BLUE << colors::WHITE << colors::BOLD
              << " SYSCOPE v" << SYSSCOPE_VERSION_STRING << " | Linux System Observability & Performance Diagnostics "
              << colors::RESET << "\n";
    std::cout << "Target OS: " << platform::get_platform_name()
              << " | Time: " << snapshot.timestamp.to_iso_string()
              << " | Active Screen: " << static_cast<int>(active_screen) << "\n";
    std::cout << "--------------------------------------------------------------------------------\n";
}

void Dashboard::render_overview(const core::MetricSnapshot& snapshot, const std::vector<analytics::Diagnosis>& diagnoses) {
    render_header(snapshot, DashboardScreen::Overview);

    // CPU Section
    double cpu_util = snapshot.cpu.total_utilization;
    const char* cpu_color = (cpu_util > 80.0) ? colors::RED : ((cpu_util > 50.0) ? colors::YELLOW : colors::GREEN);
    std::cout << colors::BOLD << "CPU UTILIZATION: " << colors::RESET
              << cpu_color << make_progress_bar(cpu_util, 24) << " "
              << util::format_metric(cpu_util) << "%" << colors::RESET
              << " (" << snapshot.cpu.per_core.size() << " Cores)\n";
    std::cout << "  User: " << util::format_metric(snapshot.cpu.user_percent) << "% | "
              << "System: " << util::format_metric(snapshot.cpu.system_percent) << "% | "
              << "IO Wait: " << util::format_metric(snapshot.cpu.iowait_percent) << "%\n\n";

    // Memory Section
    double mem_util = snapshot.memory.utilization_percent;
    const char* mem_color = (mem_util > 80.0) ? colors::RED : ((mem_util > 60.0) ? colors::YELLOW : colors::GREEN);
    std::cout << colors::BOLD << "MEMORY USAGE:    " << colors::RESET
              << mem_color << make_progress_bar(mem_util, 24) << " "
              << util::format_bytes(snapshot.memory.used_bytes) << " / "
              << util::format_bytes(snapshot.memory.total_bytes) << " ("
              << util::format_metric(mem_util) << "%)" << colors::RESET << "\n";
    std::cout << "  Available: " << util::format_bytes(snapshot.memory.available_bytes)
              << " | Cached: " << util::format_bytes(snapshot.memory.cached_bytes) << "\n\n";

    // PSI Pressure Section
    std::cout << colors::BOLD << "PRESSURE STALL INFORMATION (PSI):" << colors::RESET << "\n";
    if (snapshot.psi.has_value()) {
        std::cout << "  CPU Stall (some avg10):    " << util::format_metric(snapshot.psi->cpu_some.avg10) << "%\n";
        std::cout << "  Memory Stall (some avg10): " << util::format_metric(snapshot.psi->memory_some.avg10) << "%\n";
        std::cout << "  I/O Stall (some avg10):    " << util::format_metric(snapshot.psi->io_some.avg10) << "%\n\n";
    } else {
        std::cout << "  PSI Metrics: Unavailable\n\n";
    }

    // Top Processes Table
    std::cout << colors::BOLD << "TOP PROCESSES (By CPU %):" << colors::RESET << "\n";
    std::cout << colors::CYAN << std::left
              << std::setw(8) << "PID"
              << std::setw(20) << "NAME"
              << std::setw(10) << "CPU %"
              << std::setw(12) << "RSS MEM"
              << std::setw(8) << "STATE"
              << std::setw(8) << "THREADS"
              << colors::RESET << "\n";

    auto procs = snapshot.processes.process_list;
    std::sort(procs.begin(), procs.end(), [](const auto& a, const auto& b) {
        return a.cpu_percent > b.cpu_percent;
    });

    size_t display_count = std::min(procs.size(), static_cast<size_t>(5));
    for (size_t i = 0; i < display_count; ++i) {
        std::cout << std::left
                  << std::setw(8) << procs[i].pid
                  << std::setw(20) << procs[i].name.substr(0, 18)
                  << std::setw(10) << util::format_metric(procs[i].cpu_percent)
                  << std::setw(12) << util::format_bytes(procs[i].rss_bytes)
                  << std::setw(8) << procs[i].state
                  << std::setw(8) << procs[i].threads << "\n";
    }
    std::cout << "\n";

    // Diagnostic Alert Status
    std::cout << colors::BOLD << "SYSTEM DIAGNOSTICS & HEALTH:" << colors::RESET << "\n";
    if (diagnoses.empty()) {
        std::cout << colors::GREEN << colors::BOLD << "  ● HEALTHY [✓] " << colors::RESET
                  << "No resource contention or thermal degradation detected.\n";
    } else {
        for (const auto& diag : diagnoses) {
            std::cout << colors::RED << colors::BOLD << "  [ALERT] " << diag.condition
                      << " (Confidence: " << util::format_metric(diag.confidence * 100.0) << "%)\n"
                      << colors::RESET;
            for (const auto& ev : diag.evidence) {
                std::cout << "    • " << ev.explanation << "\n";
            }
        }
    }
    render_footer(snapshot, DashboardScreen::Overview);
}

void Dashboard::render_cpu_cores(const core::MetricSnapshot& snapshot) {
    render_header(snapshot, DashboardScreen::CpuCores);
    std::cout << colors::BOLD << "PER-CORE CPU BREAKDOWN:" << colors::RESET << "\n\n";

    for (const auto& core : snapshot.cpu.per_core) {
        const char* color = (core.total_utilization > 80.0) ? colors::RED : ((core.total_utilization > 50.0) ? colors::YELLOW : colors::GREEN);
        std::cout << "  Core " << std::setw(2) << core.core_id << " "
                  << color << make_progress_bar(core.total_utilization, 20) << " "
                  << util::format_metric(core.total_utilization) << "%" << colors::RESET
                  << " (User: " << util::format_metric(core.user_percent) << "%"
                  << " | Sys: " << util::format_metric(core.system_percent) << "%"
                  << " | Freq: " << util::format_frequency_khz(core.frequency_khz) << ")\n";
    }
    render_footer(snapshot, DashboardScreen::CpuCores);
}

void Dashboard::render_memory_disk(const core::MetricSnapshot& snapshot) {
    render_header(snapshot, DashboardScreen::MemoryAndDisk);
    std::cout << colors::BOLD << "MEMORY & SWAP ACCOUNTING:" << colors::RESET << "\n";
    std::cout << "  Total RAM:     " << util::format_bytes(snapshot.memory.total_bytes) << "\n";
    std::cout << "  Used RAM:      " << util::format_bytes(snapshot.memory.used_bytes) << "\n";
    std::cout << "  Available RAM: " << util::format_bytes(snapshot.memory.available_bytes) << "\n";
    std::cout << "  Cached RAM:    " << util::format_bytes(snapshot.memory.cached_bytes) << "\n";
    std::cout << "  Swap Total:    " << util::format_bytes(snapshot.memory.swap_total_bytes) << "\n";
    std::cout << "  Swap Used:     " << util::format_bytes(snapshot.memory.swap_used_bytes) << "\n\n";

    if (snapshot.disk.has_value() && !snapshot.disk->devices.empty()) {
        std::cout << colors::BOLD << "DISK BLOCK I/O TELEMETRY:" << colors::RESET << "\n";
        std::cout << colors::CYAN << std::left
                  << std::setw(12) << "DEVICE"
                  << std::setw(14) << "READ MB/s"
                  << std::setw(14) << "WRITE MB/s"
                  << std::setw(12) << "READ IOPS"
                  << std::setw(12) << "WRITE IOPS"
                  << std::setw(10) << "QUEUE"
                  << std::setw(10) << "UTIL %"
                  << colors::RESET << "\n";

        for (const auto& dev : snapshot.disk->devices) {
            double r_mb = dev.read_bytes_per_sec / (1024.0 * 1024.0);
            double w_mb = dev.write_bytes_per_sec / (1024.0 * 1024.0);
            std::cout << std::left
                      << std::setw(12) << dev.device_name
                      << std::setw(14) << util::format_metric(r_mb)
                      << std::setw(14) << util::format_metric(w_mb)
                      << std::setw(12) << util::format_metric(dev.read_iops, 0)
                      << std::setw(12) << util::format_metric(dev.write_iops, 0)
                      << std::setw(10) << dev.queue_depth
                      << std::setw(10) << util::format_metric(dev.io_utilization_percent) << "%\n";
        }
        std::cout << "\n";
    }

    if (snapshot.thermal.has_value()) {
        std::cout << colors::BOLD << "THERMAL SENSORS:" << colors::RESET << "\n";
        for (const auto& zone : snapshot.thermal->zones) {
            std::cout << "  " << std::setw(16) << zone.name << " (" << zone.type << "): "
                      << util::format_metric(zone.temperature_celsius) << " °C\n";
        }
    }
    render_footer(snapshot, DashboardScreen::MemoryAndDisk);
}

void Dashboard::render_processes(const core::MetricSnapshot& snapshot) {
    render_header(snapshot, DashboardScreen::Processes);
    std::cout << colors::BOLD << "PROCESS HIERARCHY LIST (Top 12 PIDs):" << colors::RESET << "\n\n";
    std::cout << colors::CYAN << std::left
              << std::setw(8) << "PID"
              << std::setw(8) << "PPID"
              << std::setw(20) << "NAME"
              << std::setw(10) << "CPU %"
              << std::setw(12) << "RSS MEM"
              << std::setw(8) << "STATE"
              << colors::RESET << "\n";

    auto procs = snapshot.processes.process_list;
    std::sort(procs.begin(), procs.end(), [](const auto& a, const auto& b) {
        return a.cpu_percent > b.cpu_percent;
    });

    size_t display_count = std::min(procs.size(), static_cast<size_t>(12));
    for (size_t i = 0; i < display_count; ++i) {
        std::cout << std::left
                  << std::setw(8) << procs[i].pid
                  << std::setw(8) << procs[i].ppid
                  << std::setw(20) << procs[i].name.substr(0, 18)
                  << std::setw(10) << util::format_metric(procs[i].cpu_percent)
                  << std::setw(12) << util::format_bytes(procs[i].rss_bytes)
                  << std::setw(8) << procs[i].state << "\n";
    }
    render_footer(snapshot, DashboardScreen::Processes);
}

void Dashboard::render_diagnostics(const core::MetricSnapshot& snapshot, const std::vector<analytics::Diagnosis>& diagnoses) {
    (void)diagnoses;
    Terminal::clear_screen();
    std::cout << colors::BG_BLUE << colors::WHITE << colors::BOLD
              << " SYSCOPE v" << SYSSCOPE_VERSION_STRING << " | Real-Time Resource Diagnostics Panel & Event History "
              << colors::RESET << "\n\n";

    if (diagnostic_history_.empty()) {
        std::cout << colors::GREEN << colors::BOLD << "● SYSTEM HEALTHY [✓]\n" << colors::RESET;
        std::cout << "All metrics operate within nominal threshold baselines. Zero resource pressure events logged.\n";
    } else {
        std::cout << colors::BOLD << "HISTORICAL RESOURCE PRESSURE EVENT LOG (Most Recent First):\n" << colors::RESET;
        for (auto it = diagnostic_history_.rbegin(); it != diagnostic_history_.rend(); ++it) {
            std::cout << "[" << it->first.to_iso_string() << "] "
                      << colors::RED << colors::BOLD << "[ALERT] " << it->second.condition << colors::RESET
                      << " (Confidence: " << util::format_metric(it->second.confidence * 100.0) << "%)\n";
            for (const auto& ev : it->second.evidence) {
                std::cout << "    • " << ev.explanation
                          << " (Contribution: +" << util::format_metric(ev.contribution * 100.0) << "%)\n";
            }
            std::cout << "\n";
        }
    }
    render_footer(snapshot, DashboardScreen::Diagnostics);
}

void Dashboard::render_footer(const core::MetricSnapshot& snapshot, DashboardScreen active_screen) {
    std::cout << "\n--------------------------------------------------------------------------------\n";
    double cpu_psi = snapshot.psi.has_value() ? snapshot.psi->cpu_some.avg10 : 0.0;
    std::cout << colors::BOLD << "Status Bar: " << colors::RESET
              << "OS: " << platform::get_platform_name()
              << " | CPU: " << util::format_metric(snapshot.cpu.total_utilization) << "%"
              << " | RAM: " << util::format_metric(snapshot.memory.utilization_percent) << "%"
              << " | PSI CPU: " << util::format_metric(cpu_psi) << "%"
              << " | Telemetry: 100ms | UI: 500ms | Queue Drops: 0\n";
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << colors::BOLD;
    std::cout << (active_screen == DashboardScreen::Overview ? colors::CYAN : colors::WHITE) << "[1] Overview " << colors::RESET;
    std::cout << (active_screen == DashboardScreen::CpuCores ? colors::CYAN : colors::WHITE) << "[2] CPU Cores " << colors::RESET;
    std::cout << (active_screen == DashboardScreen::MemoryAndDisk ? colors::CYAN : colors::WHITE) << "[3] Memory/Disk " << colors::RESET;
    std::cout << (active_screen == DashboardScreen::Processes ? colors::CYAN : colors::WHITE) << "[4] Processes " << colors::RESET;
    std::cout << (active_screen == DashboardScreen::Diagnostics ? colors::CYAN : colors::WHITE) << "[5] Diagnostics " << colors::RESET;
    std::cout << "| [r] Refresh | [q] Quit\n";
}

void Dashboard::render_snapshot(DashboardScreen screen) {
    current_screen_ = screen;

    // Collect baseline deltas
    (void)cpu_col_.collect();
    (void)proc_col_.collect();
    (void)net_col_.collect();
    (void)disk_col_.collect();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    core::MetricSnapshot snapshot{};
    snapshot.timestamp = core::Timestamp::now();

    auto cpu_res = cpu_col_.collect();
    if (cpu_res.is_ok()) snapshot.cpu = cpu_res.value();

    auto mem_res = mem_col_.collect();
    if (mem_res.is_ok()) snapshot.memory = mem_res.value();

    auto proc_res = proc_col_.collect();
    if (proc_res.is_ok()) snapshot.processes = proc_res.value();

    auto psi_res = psi_col_.collect();
    if (psi_res.is_ok()) snapshot.psi = psi_res.value();

    auto thermal_res = thermal_col_.collect();
    if (thermal_res.is_ok()) snapshot.thermal = thermal_res.value();

    auto disk_res = disk_col_.collect();
    if (disk_res.is_ok()) snapshot.disk = disk_res.value();

    auto diagnoses = correlation_engine_.analyze(snapshot);

    // Record new diagnoses into event history
    for (const auto& diag : diagnoses) {
        diagnostic_history_.push_back({snapshot.timestamp, diag});
        if (diagnostic_history_.size() > 50) {
            diagnostic_history_.erase(diagnostic_history_.begin()); // Cap history to 50 items
        }
    }

    switch (screen) {
        case DashboardScreen::Overview:
            render_overview(snapshot, diagnoses);
            break;
        case DashboardScreen::CpuCores:
            render_cpu_cores(snapshot);
            break;
        case DashboardScreen::MemoryAndDisk:
            render_memory_disk(snapshot);
            break;
        case DashboardScreen::Processes:
            render_processes(snapshot);
            break;
        case DashboardScreen::Diagnostics:
            render_diagnostics(snapshot, diagnoses);
            break;
    }
}

void Dashboard::run_interactive_loop(int refresh_rate_ms) {
    Terminal::enable_raw_mode();
    bool running = true;

    // Baseline telemetry setup
    (void)cpu_col_.collect();
    (void)proc_col_.collect();
    (void)net_col_.collect();
    (void)disk_col_.collect();

    while (running) {
        char key = Terminal::read_key();
        if (key == 'q' || key == 'Q') {
            running = false;
            break;
        } else if (key == '1') {
            current_screen_ = DashboardScreen::Overview;
        } else if (key == '2') {
            current_screen_ = DashboardScreen::CpuCores;
        } else if (key == '3') {
            current_screen_ = DashboardScreen::MemoryAndDisk;
        } else if (key == '4') {
            current_screen_ = DashboardScreen::Processes;
        } else if (key == '5') {
            current_screen_ = DashboardScreen::Diagnostics;
        }

        render_snapshot(current_screen_);
        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_rate_ms));
    }

    Terminal::disable_raw_mode();
    Terminal::clear_screen();
    std::cout << "SysScope Dashboard exited cleanly.\n";
}

} // namespace sysscope::ui
