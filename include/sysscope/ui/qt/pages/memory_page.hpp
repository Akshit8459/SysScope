#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include <QWidget>
#include <QLabel>
#include <QProgressBar>

namespace sysscope::ui::qt {

class MemoryPage : public QWidget {
    Q_OBJECT

public:
    explicit MemoryPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~MemoryPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};
    QLabel* total_ram_label_{nullptr};
    QLabel* avail_ram_label_{nullptr};
    QLabel* used_ram_label_{nullptr};
    QLabel* cached_ram_label_{nullptr};
    QLabel* swap_label_{nullptr};
    QProgressBar* ram_bar_{nullptr};
};

} // namespace sysscope::ui::qt
