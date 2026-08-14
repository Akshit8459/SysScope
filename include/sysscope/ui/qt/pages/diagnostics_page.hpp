#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include "sysscope/ui/qt/diagnosis_model.hpp"
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>

namespace sysscope::ui::qt {

class DiagnosticsPage : public QWidget {
    Q_OBJECT

public:
    explicit DiagnosticsPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~DiagnosticsPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);
    void on_incident_selected(int row);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};
    DiagnosisModel* diagnosis_model_{nullptr};

    QListWidget* incident_list_{nullptr};
    QLabel* detail_condition_{nullptr};
    QLabel* detail_severity_{nullptr};
    QLabel* detail_duration_{nullptr};
    QLabel* detail_rules_{nullptr};
    QLabel* detail_evidence_{nullptr};
};

} // namespace sysscope::ui::qt
