#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <vector>

namespace sysscope::ui::qt {

class CpuPage : public QWidget {
    Q_OBJECT

public:
    explicit CpuPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~CpuPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};
    std::vector<QProgressBar*> core_bars_;
    std::vector<QLabel*> core_labels_;
};

} // namespace sysscope::ui::qt
