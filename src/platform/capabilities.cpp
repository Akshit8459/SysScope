#include "sysscope/platform/capabilities.hpp"
#include <sstream>

namespace sysscope::platform {

std::string PlatformCapabilities::summarize() const {
    std::ostringstream ss;
    ss << "Platform Subsystem Capabilities:\n"
       << "  CPU Metrics:        " << (cpu ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Memory Metrics:     " << (memory ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Process Telemetry:  " << (process ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Network Link Stats: " << (network ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Disk Block I/O:     " << (disk ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Thermal & Hwmon:    " << (thermal ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  PSI Stalls:         " << (psi ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  NVIDIA GPU (NVML):  " << (nvidia_gpu ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Qualcomm GPU/Accel: " << (qualcomm_gpu ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  NPU Acceleration:   " << (npu ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  DSP Acceleration:   " << (dsp ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n"
       << "  Power Telemetry:    " << (power ? "AVAILABLE [✓]" : "UNAVAILABLE [-]") << "\n";
    return ss.str();
}

} // namespace sysscope::platform
