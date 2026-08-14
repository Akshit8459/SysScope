# SysScope Qt 6 Desktop Visualization Layer

## Qt Architecture & Asynchronous Model

`syscope_gui` is built using **Qt 6 Widgets** and **Qt Charts**.

```
TelemetryEngine (100ms background thread)
      │
      ▼
MetricSnapshot
      │
      ▼
QtTelemetryBridge (Queued Signal / Q_DECLARE_METATYPE)
      │
      ▼
TelemetryModel (QObject, 60s Bounded Ring Buffer)
      │
      ▼
Qt UI Widgets & QCharts (250ms GUI Thread Refresh)
```

## Bounded History Buffer

To prevent infinite GUI memory accumulation during long-running sessions, `TelemetryModel` enforces a bounded 600-sample ring buffer (60s history at 100ms sampling rate). Historical metrics (>60s) are queried on-demand from SQLite via `HistoryService`.
