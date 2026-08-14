#include "sysscope/ui/qt/pages/cpu_page.hpp"
#include "sysscope/util/format.hpp"
#include <QGridLayout>
#include <QGroupBox>

namespace sysscope::ui::qt {

CpuPage::CpuPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &CpuPage::on_snapshot_updated);
}

void CpuPage::setup_ui() {
    auto* grid_group = new QGroupBox("Per-Core Logical CPU Utilization Grid", this);
    auto* grid_layout = new QGridLayout(grid_group);

    // Initialize 16 logical core visual bars (adjusts dynamically)
    for (int i = 0; i < 16; ++i) {
        auto* label = new QLabel(QString("Core %1: 0.0%").arg(i), this);
        auto* bar = new QProgressBar(this);
        bar->setRange(0, 100);
        bar->setValue(0);

        core_labels_.push_back(label);
        core_bars_.push_back(bar);

        int row = i / 4;
        int col = (i % 4) * 2;
        grid_layout->addWidget(label, row, col);
        grid_layout->addWidget(bar, row, col + 1);
    }

    auto* main_layout = new QGridLayout(this);
    main_layout->addWidget(grid_group, 0, 0);
}

void CpuPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    size_t num_cores = snapshot.cpu.per_core.size();
    for (size_t i = 0; i < core_bars_.size(); ++i) {
        if (i < num_cores) {
            double util = snapshot.cpu.per_core[i].total_utilization;
            core_bars_[i]->setValue(static_cast<int>(util));
            core_labels_[i]->setText(QString("Core %1: %2%").arg(i).arg(util::format_metric(util).c_str()));
            core_bars_[i]->setVisible(true);
            core_labels_[i]->setVisible(true);
        } else {
            core_bars_[i]->setVisible(false);
            core_labels_[i]->setVisible(false);
        }
    }
}

} // namespace sysscope::ui::qt
