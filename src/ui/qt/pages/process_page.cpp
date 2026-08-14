#include "sysscope/ui/qt/pages/process_page.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

namespace sysscope::ui::qt {

ProcessPage::ProcessPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &ProcessPage::on_snapshot_updated);
}

void ProcessPage::setup_ui() {
    auto* main_layout = new QHBoxLayout(this);

    tree_model_ = new ProcessTreeModel(this);
    tree_view_ = new QTreeView(this);
    tree_view_->setModel(tree_model_);
    tree_view_->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tree_view_->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(tree_view_, &QTreeView::clicked, this, &ProcessPage::on_process_selected);

    auto* detail_group = new QGroupBox("Selected Process Inspector", this);
    detail_group->setFixedWidth(280);
    auto* detail_layout = new QVBoxLayout(detail_group);

    detail_pid_ = new QLabel("PID: N/A", this);
    detail_name_ = new QLabel("Name: N/A", this);
    detail_cpu_ = new QLabel("CPU Utilization: 0.0%", this);
    detail_rss_ = new QLabel("Peak RSS: 0 B", this);
    detail_threads_ = new QLabel("Threads: 0", this);
    detail_state_ = new QLabel("State: Unknown", this);

    detail_layout->addWidget(detail_pid_);
    detail_layout->addWidget(detail_name_);
    detail_layout->addWidget(detail_cpu_);
    detail_layout->addWidget(detail_rss_);
    detail_layout->addWidget(detail_threads_);
    detail_layout->addWidget(detail_state_);
    detail_layout->addStretch();

    main_layout->addWidget(tree_view_, 1);
    main_layout->addWidget(detail_group);
}

void ProcessPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    tree_model_->update_processes(snapshot.processes.process_list);
}

void ProcessPage::on_process_selected(const QModelIndex& index) {
    if (!index.isValid()) return;

    QModelIndex pid_idx = tree_model_->index(index.row(), 0, index.parent());
    QModelIndex name_idx = tree_model_->index(index.row(), 2, index.parent());
    QModelIndex cpu_idx = tree_model_->index(index.row(), 3, index.parent());
    QModelIndex rss_idx = tree_model_->index(index.row(), 4, index.parent());
    QModelIndex state_idx = tree_model_->index(index.row(), 5, index.parent());

    detail_pid_->setText(QString("PID: %1").arg(tree_model_->data(pid_idx).toString()));
    detail_name_->setText(QString("Name: %1").arg(tree_model_->data(name_idx).toString()));
    detail_cpu_->setText(QString("CPU Utilization: %1").arg(tree_model_->data(cpu_idx).toString()));
    detail_rss_->setText(QString("Peak RSS: %1").arg(tree_model_->data(rss_idx).toString()));
    detail_state_->setText(QString("State: %1").arg(tree_model_->data(state_idx).toString()));
}

} // namespace sysscope::ui::qt
