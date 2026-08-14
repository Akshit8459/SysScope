#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include "sysscope/ui/qt/chart/telemetry_chart.hpp"
#include <QWidget>
#include <QLabel>
#include <QGroupBox>

namespace sysscope::ui::qt {

class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~DashboardPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};

    QLabel* cpu_kpi_{nullptr};
    QLabel* mem_kpi_{nullptr};
    QLabel* cpu_psi_kpi_{nullptr};
    QLabel* mem_psi_kpi_{nullptr};
    QLabel* io_psi_kpi_{nullptr};
    QLabel* health_label_{nullptr};

    TelemetryChart* cpu_chart_{nullptr};
    TelemetryChart* mem_chart_{nullptr};
};

} // namespace sysscope::ui::qt
