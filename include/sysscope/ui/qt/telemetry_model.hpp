#pragma once

#include "sysscope/core/snapshot.hpp"
#include "sysscope/ui/qt/diagnosis_model.hpp"
#include <QObject>
#include <deque>
#include <mutex>
#include <vector>

namespace sysscope::ui::qt {

struct TelemetryCapabilities {
    bool thermal{false};
    bool gpu{false};
    bool npu{false};
    bool dsp{false};
    bool network{true};
    bool disk{true};
};

class TelemetryModel : public QObject {
    Q_OBJECT

public:
    explicit TelemetryModel(QObject* parent = nullptr);
    ~TelemetryModel() override = default;

    const core::MetricSnapshot& latest_snapshot() const;
    const std::deque<core::MetricSnapshot>& history_buffer() const;
    const TelemetryCapabilities& capabilities() const;

public slots:
    void update_snapshot(const sysscope::core::MetricSnapshot& snapshot);
    void raise_diagnosis(const sysscope::ui::qt::DiagnosisEvent& event);

signals:
    void snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);
    void diagnosis_raised(const sysscope::ui::qt::DiagnosisEvent& event);

private:
    mutable std::mutex mutex_;
    core::MetricSnapshot latest_{};
    std::deque<core::MetricSnapshot> history_buffer_; // Bounded ring buffer (600 samples max = 60s)
    TelemetryCapabilities capabilities_{};
    static constexpr size_t MAX_RING_BUFFER_SIZE = 600;
};

} // namespace sysscope::ui::qt
