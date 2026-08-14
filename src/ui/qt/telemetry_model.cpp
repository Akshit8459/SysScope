#include "sysscope/ui/qt/telemetry_model.hpp"

namespace sysscope::ui::qt {

TelemetryModel::TelemetryModel(QObject* parent)
    : QObject(parent) {}

const core::MetricSnapshot& TelemetryModel::latest_snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_;
}

const std::deque<core::MetricSnapshot>& TelemetryModel::history_buffer() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return history_buffer_;
}

const TelemetryCapabilities& TelemetryModel::capabilities() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return capabilities_;
}

void TelemetryModel::update_snapshot(const sysscope::core::MetricSnapshot& snapshot) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_ = snapshot;
        history_buffer_.push_back(snapshot);
        if (history_buffer_.size() > MAX_RING_BUFFER_SIZE) {
            history_buffer_.pop_front();
        }
    }
    emit snapshot_updated(snapshot);
}

void TelemetryModel::raise_diagnosis(const sysscope::ui::qt::DiagnosisEvent& event) {
    emit diagnosis_raised(event);
}

} // namespace sysscope::ui::qt
