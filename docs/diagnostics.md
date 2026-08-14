# SysScope Diagnostic Correlation Engine

## Cross-Resource Diagnostic Rule Matrix

The `CorrelationEngine` analyzes multi-dimensional telemetry snapshots to detect system bottlenecks and resource contention scenarios.

```
CPU Utilization > 75%  AND  CPU PSI > 5.0%    ──►  CpuSchedulingContention
RAM Available < 10%    AND  Memory PSI > 2.0% ──►  MemoryPressure
Disk Util > 80%        AND  I/O PSI > 4.0%    ──►  IoBottleneck
Temp > 85.0 °C         AND  Freq Drop         ──►  ThermalThrottling
```

## Structured `DiagnosisEvent` Pipeline

Diagnostic alerts emit structured `DiagnosisEvent` instances containing:
- `start_time` & `end_time` timestamps
- Strongly-typed `DiagnosisType` enum
- `Severity` level (`Low`, `Medium`, `High`, `Critical`)
- `triggered_rules` array
- Raw `evidence` vector (observed value, threshold, contribution score, human-readable explanation)
