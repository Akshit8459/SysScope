# SysScope Architecture & Design Principles

## Overview
SysScope is structured around a strict separation between high-frequency telemetry acquisition, diagnostic correlation, persistence, and presentation.

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

## Architectural Isolation Principles

1. **Zero Qt Dependency in Core Library (`libsysscope_core.a`)**:
   - The core library contains zero Qt header inclusions, symbols, or linkages (`nm libsysscope_core.a | grep -i qt` $\rightarrow$ 0 matches).
   - Core collectors and telemetry pipelines can be deployed in headless embedded environments, daemons, or CLI tools without dragging in GUI libraries.

2. **Presentation Adapters (`QtTelemetryBridge`)**:
   - Qt GUI presentation layers (`src/ui/qt/`) receive snapshots through `QtTelemetryBridge`, which uses Qt's thread-safe queued signal dispatch system.
