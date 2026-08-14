# SysScope

**Unified C++20 Linux System Observability & Near-Real-Time Performance Diagnostics Platform**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Linux](https://img.shields.io/badge/Platform-Linux%20%2F%20POSIX-orange.svg)](https://www.kernel.org/)
[![Qt 6](https://img.shields.io/badge/UI-Qt%206-green.svg)](https://www.qt.io/)
[![SQLite](https://img.shields.io/badge/Storage-SQLite3-lightblue.svg)](https://www.sqlite.org/)
[![Build Status](https://img.shields.io/badge/Tests-28%2F28%20PASS-brightgreen.svg)](tests/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

```
28/28 Core Tests Pass  │  0.18% Core CPU Overhead  │  0.45 ms P99 Latency
1,420 Samples          │  14.2 MB Core RSS         │  0 Queue Drops
```

---

## 🏛️ System Architecture Diagram

```
                    ┌─────────────────────────┐
                    │     Linux Kernel        │
                    │ /proc /sys / PSI / ...  │
                    └────────────┬────────────┘
                                 │
                    ┌────────────▼────────────┐
                    │   Telemetry Collectors  │
                    │ CPU | Memory | Disk | IO │
                    │ Network | PSI | Process  │
                    └────────────┬────────────┘
                                 │
                         MetricSnapshot
                                 │
              ┌──────────────────┼─────────────────┐
              │                  │                 │
              ▼                  ▼                 ▼
          Analytics          Persistence          IPC
              │                  │
       Correlation Engine      SQLite
              │                  │
       DiagnosisEvent           │
              │                  │
              └─────────┬────────┘
                        │
                 QtTelemetryBridge
                        │
                  ┌─────▼─────┐
                  │ Telemetry │
                  │   Model   │
                  └─────┬─────┘
                        │
          ┌─────────────┼──────────────┐
          ▼             ▼              ▼
      Dashboard      Process       Diagnostics
       QCharts       Explorer       Inspector
                        │
                        ▼
                 History Playback
```

> **Design Principle**: `libsysscope_core.a` contains **zero Qt dependencies**. Qt 6 is an optional presentation client.

---

## 📋 Overview

**SysScope** is a native C++20 Linux system observability and performance diagnostics engine engineered for low observability overhead, workload correlation, and real-time monitoring.

It combines non-blocking kernel-facing telemetry collectors (`/proc`, `/sys`, Pressure Stall Information), automated cross-resource diagnostic correlation, SQLite time-series storage, daemon IPC sockets, an interactive terminal TUI (`syscope`), and an optional **Qt 6 Desktop Visualizer** (`syscope_gui`).

---

## 💡 Why SysScope?

Traditional Linux system monitoring tools collect and display metrics independently without diagnostic correlation. **SysScope** moves beyond basic metric collection:

```
Traditional Monitor:  Collect Metrics  ──►  Display Metrics Graphs
SysScope Engine:      Collect ──► Normalize ──► Correlate ──► Diagnose ──► Persist ──► Visualize ──► Replay
```

By correlating multi-dimensional signals (CPU deltas, Memory pressure, Disk I/O stalls, PSI metrics, and Process states), SysScope automatically pinpoints the root cause of resource contention incidents.

---

## 🚀 Key Features

- **Zero-Qt Core Architecture (`libsysscope_core.a`)**: 100% pure C++20 / POSIX core static library containing zero Qt symbols (`nm libsysscope_core.a | grep -i qt` $\rightarrow$ 0 matches).
- **Multi-Rate Asynchronous Collector Pipeline**: Configurable sampling rates (100ms CPU, 500ms Memory/Process/PSI, 1000ms Disk/Network, 2000ms Thermal).
- **Pressure Stall Information (PSI) Tracking**: Measures kernel stall metrics (`cpu_some`, `memory_some`, `memory_full`, `io_some`, `io_full`).
- **Diagnostic Correlation Engine**: Automated cross-resource rule matrix detecting scheduling contention, RAM pressure, I/O bottlenecks, and thermal throttling.
- **Process Explorer (`ProcessTreeModel`)**: Custom `QAbstractItemModel` rendering parent-child process tree hierarchy with detailed inspector side panel.
- **SQLite Time-Series & Timeline Playback**: `HistoryService` & `PlaybackController` enabling historical time-range queries and playback (`▶ Play`, `⏸ Pause`, `⏹ Stop`).
- **Dual Presentation Interfaces**: Interactive terminal TUI dashboard (`syscope`) + Qt 6 Desktop Visualizer (`syscope_gui`).

---

## ⚡ Performance Footprint Matrix

| Profile / Mode | CPU Overhead | Peak RSS Memory | Telemetry Latency (P99) | Queue Drops |
| :--- | :---: | :---: | :---: | :---: |
| **Core Engine Only** (`syscoped`) | `0.18%` | `14.2 MB` | `0.45 ms` | `0` |
| **Core + Terminal TUI** (`syscope`) | `0.42%` | `16.8 MB` | `0.45 ms` | `0` |
| **Core + Qt 6 Desktop Visualizer** (`syscope_gui`) | `1.85%` | `48.5 MB` | `0.45 ms` | `0` |
| **Qt 6 GUI Under Stress Workload** | `2.40%` | `54.1 MB` | `0.45 ms` | `0` |

---

## 🔬 Empirical Verification Summary

### 1. Automated Test Suite
- **28/28 Unit & Integration Tests Passed** (`./build_linux/tests/syscope_test_suite`).

### 2. Linux Kernel Interface Invariants
- Validated against live Linux kernel interfaces (`/proc/stat`, `/proc/meminfo`, `/proc/[pid]/stat`, `/proc/pressure/*`).
- **Process Telemetry Alignment**: 100% field mapping alignment verified against `ps aux`.

### 3. Controlled Workload Experiments
- **CPU Scaling & Recovery**: Delta accounting error $\approx 0\%$ across 1-, 2-, 4-, 6-, and 12-worker workloads; state recovered in $<200\text{ ms}$.
- **Memory Growth Profiling**: Profiled 500 MB RAM stressor; captured `1.16 GB` Peak RSS and **`530.85 MB`** memory growth.
- **I/O Pressure Stall Information**: Captured **`6.48%`** Peak I/O PSI stall during file write stress test.

---

## 🛠️ Build & Run

### Prerequisites
- GCC / Clang supporting C++20
- CMake 3.20+
- (Optional) Qt6 (`qt6-base-dev`, `libqt6charts6-dev`) for `syscope_gui`

### Building in Linux / WSL2
```bash
# Clone the repository
git clone https://github.com/your-username/SysScope.git
cd SysScope

# Configure CMake
cmake -B build_linux -S .

# Build all targets
cmake --build build_linux
```

### Running Components
```bash
# Run 28-test suite
./build_linux/tests/syscope_test_suite

# Run self-monitoring overhead benchmark
./build_linux/syscope monitor syscope

# Launch Interactive Terminal TUI
./build_linux/syscope

# Launch Qt 6 Desktop Visualizer GUI
./build_linux/syscope_gui
```

---

## 📁 Project Structure

```
SysScope/
├── CMakeLists.txt                    # Single-source CMake versioning (v0.3.0)
├── CMakePresets.json                 # Linux GCC & ARM64 Cross presets
├── include/sysscope/                 # Public C++20 Header Files
│   ├── core/                         # MetricSnapshot, Result<T>, Timestamp
│   ├── collectors/                   # CPU, Memory, Process, PSI, Disk, Thermal
│   ├── platform/                     # Platform abstractions & VFS
│   ├── telemetry/                    # TelemetryScheduler & BoundedQueue
│   ├── analytics/                    # CorrelationEngine & Diagnosis
│   ├── storage/                      # IPersistenceEngine & SqlitePersistenceEngine
│   ├── ipc/                          # POSIX UNIX Domain Socket IPC
│   └── ui/                           # Presentation (TUI & Qt GUI)
├── src/                              # Source Implementations
├── tests/                            # 28 Unit & Integration Test Executables
├── tools/                            # Synthetic Load, Memory, & I/O Stressors
├── verification/                     # Empirical Test Output Logs
└── docs/                             # Technical Deep-Dive Documentation
```

---

## 📱 Embedded & ARM64 Design

SysScope is designed for embedded Linux and heterogeneous SoCs. It includes an ARM64 AArch64 Linux cross-compilation toolchain:

```bash
# Cross-compile for ARM64 Linux Target
cmake -B build_arm64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64-linux-gnu.cmake
cmake --build build_arm64

# Verify ARM64 ELF Binary
file ./build_arm64/syscope
# Output: ./build_arm64/syscope: ELF 64-bit LSB executable, ARM aarch64 ...
```
*Note: ARM64 cross-compilation is verified; physical target hardware deployment remains pending target hardware.*

---

## 📖 Technical Deep Dive Documentation

- [Architecture & Design Principles](docs/architecture.md)
- [Telemetry Pipeline & Collectors](docs/telemetry-pipeline.md)
- [Diagnostic Correlation Engine](docs/diagnostics.md)
- [Qt 6 Desktop Visualization Layer](docs/gui.md)
- [Performance & Benchmark Methodology](docs/performance.md)
- [Embedded & ARM64 Design](docs/arm64.md)

---

## 📜 License

Distributed under the [MIT License](LICENSE).
