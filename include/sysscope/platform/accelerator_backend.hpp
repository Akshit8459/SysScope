#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include <string>

namespace sysscope::platform {

class IAcceleratorBackend {
public:
    virtual ~IAcceleratorBackend() = default;

    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual bool is_available() const = 0;
    [[nodiscard]] virtual core::Result<core::GpuSnapshot> collect() = 0;
};

class MockGpuBackend : public IAcceleratorBackend {
public:
    MockGpuBackend(bool available = true) : available_(available) {}
    ~MockGpuBackend() override = default;

    [[nodiscard]] std::string name() const override { return "Mock Accelerator Backend"; }
    [[nodiscard]] bool is_available() const override { return available_; }
    
    void set_snapshot(core::GpuSnapshot snap) { snapshot_ = snap; }

    [[nodiscard]] core::Result<core::GpuSnapshot> collect() override {
        if (!available_) {
            return core::Result<core::GpuSnapshot>::error("GPU Backend Unavailable");
        }
        return core::Result<core::GpuSnapshot>(snapshot_);
    }

private:
    bool available_{true};
    core::GpuSnapshot snapshot_{"Mock GPU", 45.0, 30.0, 3221225472ULL, 8589934592ULL, 62.0, 75.0};
};

class QualcommAcceleratorBackend : public IAcceleratorBackend {
public:
    QualcommAcceleratorBackend() = default;
    ~QualcommAcceleratorBackend() override = default;

    [[nodiscard]] std::string name() const override { return "Qualcomm Adreno/Hexagon NPU/DSP Backend"; }
    [[nodiscard]] bool is_available() const override { return false; } // Hardware target abstraction

    [[nodiscard]] core::Result<core::GpuSnapshot> collect() override {
        return core::Result<core::GpuSnapshot>::error("Qualcomm hardware acceleration requires target SoC environment.");
    }
};

class NvidiaGpuBackend : public IAcceleratorBackend {
public:
    NvidiaGpuBackend() = default;
    ~NvidiaGpuBackend() override = default;

    [[nodiscard]] std::string name() const override { return "NVIDIA NVML Backend"; }
    [[nodiscard]] bool is_available() const override { return false; } // NVML target abstraction

    [[nodiscard]] core::Result<core::GpuSnapshot> collect() override {
        return core::Result<core::GpuSnapshot>::error("NVML library unavailable on host environment.");
    }
};

} // namespace sysscope::platform
