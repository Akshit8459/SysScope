#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include "sysscope/ui/qt/process_tree_model.hpp"
#include <QWidget>
#include <QTreeView>
#include <QLabel>
#include <QGroupBox>

namespace sysscope::ui::qt {

class ProcessPage : public QWidget {
    Q_OBJECT

public:
    explicit ProcessPage(TelemetryModel* model, QWidget* parent = nullptr);
    ~ProcessPage() override = default;

public slots:
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);
    void on_process_selected(const QModelIndex& index);

private:
    void setup_ui();

    TelemetryModel* model_{nullptr};
    ProcessTreeModel* tree_model_{nullptr};

    QTreeView* tree_view_{nullptr};
    QLabel* detail_pid_{nullptr};
    QLabel* detail_name_{nullptr};
    QLabel* detail_cpu_{nullptr};
    QLabel* detail_rss_{nullptr};
    QLabel* detail_threads_{nullptr};
    QLabel* detail_state_{nullptr};
};

} // namespace sysscope::ui::qt
