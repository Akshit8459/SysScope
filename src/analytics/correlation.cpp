#include "sysscope/analytics/correlation.hpp"
#include "sysscope/util/format.hpp"
#include <algorithm>

namespace sysscope::analytics {

std::vector<Diagnosis> CorrelationEngine::analyze(const core::MetricSnapshot& snapshot) const {
    std::vector<Diagnosis> diagnoses;

    // Rule 1: Thermal Throttling Diagnosis
    if (snapshot.gpu.has_value() || !snapshot.cpu.per_core.empty()) {
        double max_temp = 0.0;
        if (snapshot.gpu.has_value()) {
            max_temp = std::max(max_temp, snapshot.gpu->temperature_celsius);
        }
        if (snapshot.thermal.has_value()) {
            for (const auto& zone : snapshot.thermal->zones) {
                max_temp = std::max(max_temp, zone.temperature_celsius);
            }
        }

        if (max_temp > 75.0) {
            Diagnosis thermal_diag{};
            thermal_diag.condition = "THERMAL THROTTLING";
            double confidence = 0.0;

            Evidence temp_ev{};
            temp_ev.metric = core::MetricType::Temperature;
            temp_ev.observed_value = max_temp;
            temp_ev.threshold = 75.0;
            temp_ev.contribution = 0.40;
            temp_ev.explanation = "Peak temperature (" + util::format_metric(max_temp) + "°C) exceeded threshold 75°C";
            thermal_diag.evidence.push_back(temp_ev);
            confidence += 0.40;

            if (snapshot.cpu.total_utilization > 80.0) {
                Evidence cpu_ev{};
                cpu_ev.metric = core::MetricType::CpuUtilization;
                cpu_ev.observed_value = snapshot.cpu.total_utilization;
                cpu_ev.threshold = 80.0;
                cpu_ev.contribution = 0.25;
                cpu_ev.explanation = "CPU utilization remained >80%";
                thermal_diag.evidence.push_back(cpu_ev);
                confidence += 0.25;
            }

            if (snapshot.gpu.has_value() && snapshot.gpu->utilization_percent > 85.0) {
                Evidence gpu_ev{};
                gpu_ev.metric = core::MetricType::GpuUtilization;
                gpu_ev.observed_value = snapshot.gpu->utilization_percent;
                gpu_ev.threshold = 85.0;
                gpu_ev.contribution = 0.25;
                gpu_ev.explanation = "GPU utilization remained >85%";
                thermal_diag.evidence.push_back(gpu_ev);
                confidence += 0.25;
            }

            thermal_diag.confidence = std::min(1.0, confidence);
            diagnoses.push_back(thermal_diag);
        }
    }

    // Rule 2: Memory Pressure Contention
    if (snapshot.psi.has_value() && snapshot.memory.utilization_percent > 70.0) {
        if (snapshot.psi->memory_some.avg10 > 5.0) {
            Diagnosis mem_diag{};
            mem_diag.condition = "MEMORY CONTENTION & PRESSURE";
            double confidence = 0.0;

            Evidence psi_ev{};
            psi_ev.metric = core::MetricType::PsiMemorySome;
            psi_ev.observed_value = snapshot.psi->memory_some.avg10;
            psi_ev.threshold = 5.0;
            psi_ev.contribution = 0.50;
            psi_ev.explanation = "Linux Memory PSI (some avg10=" + util::format_metric(snapshot.psi->memory_some.avg10) + "%) indicates task stall";
            mem_diag.evidence.push_back(psi_ev);
            confidence += 0.50;

            Evidence mem_ev{};
            mem_ev.metric = core::MetricType::MemoryUtilization;
            mem_ev.observed_value = snapshot.memory.utilization_percent;
            mem_ev.threshold = 70.0;
            mem_ev.contribution = 0.35;
            mem_ev.explanation = "RAM utilization high (" + util::format_metric(snapshot.memory.utilization_percent) + "%)";
            mem_diag.evidence.push_back(mem_ev);
            confidence += 0.35;

            mem_diag.confidence = std::min(1.0, confidence);
            diagnoses.push_back(mem_diag);
        }
    }

    // Rule 3: I/O Saturation & Bottleneck
    if (snapshot.psi.has_value() && snapshot.psi->io_some.avg10 > 5.0) {
        Diagnosis io_diag{};
        io_diag.condition = "I/O BOTTLENECK";
        double confidence = 0.0;

        Evidence io_ev{};
        io_ev.metric = core::MetricType::PsiIoSome;
        io_ev.observed_value = snapshot.psi->io_some.avg10;
        io_ev.threshold = 5.0;
        io_ev.contribution = 0.55;
        io_ev.explanation = "Linux I/O PSI (some avg10=" + util::format_metric(snapshot.psi->io_some.avg10) + "%) indicates task wait stall";
        io_diag.evidence.push_back(io_ev);
        confidence += 0.55;

        if (snapshot.cpu.iowait_percent > 10.0) {
            Evidence iowait_ev{};
            iowait_ev.metric = core::MetricType::CpuUtilization;
            iowait_ev.observed_value = snapshot.cpu.iowait_percent;
            iowait_ev.threshold = 10.0;
            iowait_ev.contribution = 0.35;
            iowait_ev.explanation = "CPU I/O Wait elevated (" + util::format_metric(snapshot.cpu.iowait_percent) + "%)";
            io_diag.evidence.push_back(iowait_ev);
            confidence += 0.35;
        }

        io_diag.confidence = std::min(1.0, confidence);
        diagnoses.push_back(io_diag);
    }

    // Rule 4: CPU Scheduling Contention
    if (snapshot.psi.has_value() && snapshot.cpu.total_utilization > 75.0 && snapshot.psi->cpu_some.avg10 > 5.0) {
        Diagnosis cpu_diag{};
        cpu_diag.condition = "CPU SCHEDULING CONTENTION";
        double confidence = 0.0;

        Evidence psi_ev{};
        psi_ev.metric = core::MetricType::PsiCpuSome;
        psi_ev.observed_value = snapshot.psi->cpu_some.avg10;
        psi_ev.threshold = 5.0;
        psi_ev.contribution = 0.50;
        psi_ev.explanation = "Linux CPU PSI (some avg10=" + util::format_metric(snapshot.psi->cpu_some.avg10) + "%) indicates runnable task stall";
        cpu_diag.evidence.push_back(psi_ev);
        confidence += 0.50;

        Evidence util_ev{};
        util_ev.metric = core::MetricType::CpuUtilization;
        util_ev.observed_value = snapshot.cpu.total_utilization;
        util_ev.threshold = 75.0;
        util_ev.contribution = 0.40;
        util_ev.explanation = "High CPU utilization (" + util::format_metric(snapshot.cpu.total_utilization) + "%) coincides with scheduling contention";
        cpu_diag.evidence.push_back(util_ev);
        confidence += 0.40;

        cpu_diag.confidence = std::min(1.0, confidence);
        diagnoses.push_back(cpu_diag);
    }

    return diagnoses;
}

} // namespace sysscope::analytics
