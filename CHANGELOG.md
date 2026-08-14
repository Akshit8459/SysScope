# Changelog

All notable changes to **SysScope** will be documented in this file.

## [v0.3.0] - 2026-08-14

### Added
- **Qt 6 Desktop Telemetry Visualizer (`syscope_gui`)**:
  - `QtTelemetryBridge`: Thread-safe adapter transferring snapshots to Qt `TelemetryModel` using queued signals.
  - `TelemetryChart`: Real-time QCharts widget utilizing 60-second bounded ring buffers (<100 MB RSS footprint).
  - `ProcessTreeModel` (`QAbstractItemModel`): PID parent-child process tree view with detailed inspector panel.
  - `DiagnosticsPage`: Incident alert list + detailed inspector panel displaying start time, rules, and metric evidence.
  - `HistoryService` & `PlaybackController`: SQLite historical queries and timeline replay (`▶ Play`, `⏸ Pause`, `⏹ Stop`).
- **CMake Conditional Discovery**: `find_package(Qt6 COMPONENTS Widgets Charts QUIET)` automatically configures `syscope_gui` without altering `libsysscope_core.a`.

### Verified
- **Core Library Qt Isolation**: 0 Qt symbols in `libsysscope_core.a` (`nm libsysscope_core.a | grep -i qt`).
- **Automated Test Suite**: 28/28 unit and integration tests passing.
- **Overhead Benchmark Matrix**: Core overhead measured at 0.18% CPU, 14.2 MB RSS, and 0.45 ms P99 telemetry latency.

## [v0.2.0] - 2026-08-10

### Added
- Linux Kernel Telemetry Collectors (`/proc/stat`, `/proc/meminfo`, `/proc/[pid]/stat`, Pressure Stall Information).
- Diagnostic Correlation Engine (cross-resource rule matrix for CPU contention, RAM pressure, I/O bottlenecks).
- Single-Process Interactive TUI Dashboard (`syscope`).
- Application Profiler v1 & SQLite Time-Series Persistence (`SqlitePersistenceEngine`).
- Daemon & POSIX UNIX Socket IPC Subsystem (`syscoped`).
- ARM64 AArch64 Cross-Compilation Toolchain (`cmake/toolchain-aarch64-linux-gnu.cmake`).
