#include "sysscope/analytics/profiler.hpp"
#include "sysscope/platform/platform.hpp"
#include "sysscope/util/format.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <vector>

namespace sysscope::analytics {

ApplicationProfiler::ApplicationProfiler(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

ProfileReport ApplicationProfiler::profile_command(const std::string& command, int sample_interval_ms) {
    ProfileReport report{};
    report.command = command;

    if (!platform::is_linux_platform()) {
        report.summary_diagnosis = "WARNING: Native Linux kernel VFS (/proc, /sys) telemetry unavailable on " + 
                                   platform::get_platform_name() + ". Profiler monitored process execution duration only.";
    }

    collectors::CpuCollector cpu_col(fs_reader_);
    collectors::MemoryCollector mem_col(fs_reader_);
    collectors::PsiCollector psi_col(fs_reader_);

    // Baseline sample
    (void)cpu_col.collect();
    auto initial_mem = mem_col.collect();
    if (initial_mem.is_ok()) {
        report.initial_rss_bytes = initial_mem.value().used_bytes;
    }

    std::atomic<bool> workload_active{true};
    std::vector<double> cpu_samples;
    std::vector<uint64_t> rss_samples;
    std::vector<double> psi_cpu_samples;
    std::vector<double> psi_mem_samples;
    std::vector<double> psi_io_samples;

    // Asynchronous 100ms sampling thread
    std::thread sampler([&]() {
        while (workload_active.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sample_interval_ms));
            if (!workload_active.load()) break;

            auto c_res = cpu_col.collect();
            if (c_res.is_ok()) {
                cpu_samples.push_back(c_res.value().total_utilization);
            }

            auto m_res = mem_col.collect();
            if (m_res.is_ok()) {
                rss_samples.push_back(m_res.value().used_bytes);
            }

            auto p_res = psi_col.collect();
            if (p_res.is_ok()) {
                psi_cpu_samples.push_back(p_res.value().cpu_some.avg10);
                psi_mem_samples.push_back(p_res.value().memory_some.avg10);
                psi_io_samples.push_back(p_res.value().io_some.avg10);
            }
        }
    });

    auto start_time = std::chrono::steady_clock::now();

    // Execute target command synchronously
    int exit_code = std::system(command.c_str());
    (void)exit_code;

    auto end_time = std::chrono::steady_clock::now();
    workload_active.store(false);
    if (sampler.joinable()) {
        sampler.join();
    }

    report.duration_seconds = std::chrono::duration<double>(end_time - start_time).count();

    // Final sample post-execution fallback if fast workload
    if (cpu_samples.empty()) {
        auto post_cpu = cpu_col.collect();
        if (post_cpu.is_ok()) cpu_samples.push_back(post_cpu.value().total_utilization);
    }
    if (rss_samples.empty()) {
        auto post_mem = mem_col.collect();
        if (post_mem.is_ok()) rss_samples.push_back(post_mem.value().used_bytes);
    }

    // Process aggregated metrics
    if (!cpu_samples.empty()) {
        double sum = 0.0;
        double peak = 0.0;
        for (double val : cpu_samples) {
            sum += val;
            peak = std::max(peak, val);
        }
        report.avg_cpu_percent = sum / static_cast<double>(cpu_samples.size());
        report.peak_cpu_percent = peak;
    }

    if (!rss_samples.empty()) {
        uint64_t max_rss = report.initial_rss_bytes;
        for (uint64_t val : rss_samples) {
            max_rss = std::max(max_rss, val);
        }
        report.peak_rss_bytes = max_rss;
        if (report.peak_rss_bytes > report.initial_rss_bytes) {
            report.memory_growth_bytes = report.peak_rss_bytes - report.initial_rss_bytes;
        }
    }

    if (!psi_cpu_samples.empty()) {
        report.peak_psi_cpu_some = *std::max_element(psi_cpu_samples.begin(), psi_cpu_samples.end());
    }
    if (!psi_mem_samples.empty()) {
        report.peak_psi_memory_some = *std::max_element(psi_mem_samples.begin(), psi_mem_samples.end());
    }
    if (!psi_io_samples.empty()) {
        report.peak_psi_io_some = *std::max_element(psi_io_samples.begin(), psi_io_samples.end());
    }

    if (platform::is_linux_platform()) {
        if (report.peak_psi_memory_some > 10.0) {
            report.summary_diagnosis = "Memory-bound workload with severe PSI stall pressure.";
        } else if (report.peak_psi_io_some > 10.0) {
            report.summary_diagnosis = "I/O-bound workload with elevated disk wait pressure.";
        } else if (report.peak_psi_cpu_some > 10.0 && report.avg_cpu_percent > 70.0) {
            report.summary_diagnosis = "CPU-bound workload with task scheduling contention.";
        } else if (report.avg_cpu_percent > 85.0) {
            report.summary_diagnosis = "CPU-bound workload operating near full compute capacity.";
        } else {
            report.summary_diagnosis = "Balanced resource execution.";
        }
    }

    return report;
}

} // namespace sysscope::analytics
