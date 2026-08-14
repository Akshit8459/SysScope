#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include <QWidget>
#include <QLabel>

namespace sysscope::ui::qt {

class PsiPage : public QWidget {
    Q_OBJECT

public:
    explicit PsiPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~PsiPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};
    QLabel* cpu_some_label_{nullptr};
    QLabel* mem_some_label_{nullptr};
    QLabel* mem_full_label_{nullptr};
    QLabel* io_some_label_{nullptr};
    QLabel* io_full_label_{nullptr};
};

} // namespace sysscope::ui::qt
