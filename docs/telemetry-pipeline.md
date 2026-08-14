# SysScope Telemetry Pipeline & Collector Architecture

## Multi-Rate Telemetry Scheduler

Telemetry acquisition runs asynchronously on dedicated background threads managed by `TelemetryScheduler` and pushed to a thread-safe `BoundedQueue<MetricSnapshot>`.

| Collector | Sampling Interval | Linux Interface Source |
| :--- | :---: | :--- |
| **CPU Collector** | `100 ms` | `/proc/stat` (per-core & aggregate Jiffies deltas) |
| **Memory Collector** | `500 ms` | `/proc/meminfo` (`MemAvailable` priority calculation) |
| **Process Collector** | `500 ms` | `/proc/[pid]/stat`, `/proc/[pid]/cmdline` |
| **PSI Collector** | `500 ms` | `/proc/pressure/cpu`, `/proc/pressure/memory`, `/proc/pressure/io` |
| **Disk Block Collector** | `1000 ms` | `/proc/diskstats`, `/sys/block/` |
| **Network Collector** | `1000 ms` | `/proc/net/dev`, POSIX `rtnetlink` sockets |
| **Thermal Collector** | `2000 ms` | `/sys/class/thermal/thermal_zone*/` |

## Delta Accounting Formula

CPU utilization is computed strictly over time deltas ($\Delta t = t_2 - t_1$):

$$\text{Utilization}\% = \left( 1 - \frac{\Delta \text{Idle}}{\Delta \text{Total}} \right) \times 100$$

This guarantees robust accounting across dynamic frequency scaling (DVFS) and multi-core systems.
