#pragma once

#include "sysscope/core/snapshot.hpp"
#include <QObject>
#include <memory>

namespace sysscope::ui::qt {

class TelemetryModel;

class QtTelemetryBridge : public QObject {
    Q_OBJECT

public:
    explicit QtTelemetryBridge(TelemetryModel* model, QObject* parent = nullptr);
    ~QtTelemetryBridge() override = default;

    void post_snapshot(const sysscope::core::MetricSnapshot& snapshot);

signals:
    void snapshot_dispatched(const sysscope::core::MetricSnapshot& snapshot);

private:
    TelemetryModel* model_{nullptr};
};

} // namespace sysscope::ui::qt
