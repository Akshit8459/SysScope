#include "sysscope/ui/qt/telemetry_bridge.hpp"
#include "sysscope/ui/qt/telemetry_model.hpp"

namespace sysscope::ui::qt {

QtTelemetryBridge::QtTelemetryBridge(TelemetryModel* model, QObject* parent)
    : QObject(parent), model_(model) {
    qRegisterMetaType<sysscope::core::MetricSnapshot>("sysscope::core::MetricSnapshot");
    connect(this, &QtTelemetryBridge::snapshot_dispatched,
            model_, &TelemetryModel::update_snapshot,
            Qt::QueuedConnection);
}

void QtTelemetryBridge::post_snapshot(const sysscope::core::MetricSnapshot& snapshot) {
    emit snapshot_dispatched(snapshot);
}

} // namespace sysscope::ui::qt
