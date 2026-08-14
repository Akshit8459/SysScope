#include "sysscope/ui/qt/pages/dashboard_page.hpp"
#include "sysscope/util/format.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

namespace sysscope::ui::qt {

DashboardPage::DashboardPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &DashboardPage::on_snapshot_updated);
}

void DashboardPage::setup_ui() {
    auto* main_layout = new QVBoxLayout(this);

    // KPI Cards Header Grid
    auto* kpi_group = new QGroupBox("System Key Performance Indicators (KPI)", this);
    auto* kpi_layout = new QGridLayout(kpi_group);

    cpu_kpi_ = new QLabel("CPU: 0.0%", this);
    mem_kpi_ = new QLabel("RAM: 0.0%", this);
    cpu_psi_kpi_ = new QLabel("CPU PSI: 0.00%", this);
    mem_psi_kpi_ = new QLabel("MEM PSI: 0.00%", this);
    io_psi_kpi_ = new QLabel("IO PSI: 0.00%", this);
    health_label_ = new QLabel("System Health: ✔ OPTIMAL", this);

    QFont font = cpu_kpi_->font();
    font.setBold(true);
    font.setPointSize(12);

    cpu_kpi_->setFont(font);
    mem_kpi_->setFont(font);
    cpu_psi_kpi_->setFont(font);
    mem_psi_kpi_->setFont(font);
    io_psi_kpi_->setFont(font);

    kpi_layout->addWidget(cpu_kpi_, 0, 0);
    kpi_layout->addWidget(mem_kpi_, 0, 1);
    kpi_layout->addWidget(cpu_psi_kpi_, 0, 2);
    kpi_layout->addWidget(mem_psi_kpi_, 1, 0);
    kpi_layout->addWidget(io_psi_kpi_, 1, 1);
    kpi_layout->addWidget(health_label_, 1, 2);

    main_layout->addWidget(kpi_group);

    // Charts Layout
    auto* charts_layout = new QHBoxLayout();
    cpu_chart_ = new TelemetryChart("CPU Utilization Timeline", "% Utilization", 100.0, this);
    mem_chart_ = new TelemetryChart("Memory Utilization Timeline", "% Utilization", 100.0, this);

    charts_layout->addWidget(cpu_chart_);
    charts_layout->addWidget(mem_chart_);

    main_layout->addLayout(charts_layout);
}

void DashboardPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    cpu_kpi_->setText(QString("CPU: %1%").arg(util::format_metric(snapshot.cpu.total_utilization).c_str()));
    mem_kpi_->setText(QString("RAM: %1%").arg(util::format_metric(snapshot.memory.utilization_percent).c_str()));
    
    if (snapshot.psi.has_value()) {
        cpu_psi_kpi_->setText(QString("CPU PSI: %1%").arg(util::format_metric(snapshot.psi->cpu_some.avg10).c_str()));
        mem_psi_kpi_->setText(QString("MEM PSI: %1%").arg(util::format_metric(snapshot.psi->memory_some.avg10).c_str()));
        io_psi_kpi_->setText(QString("IO PSI: %1%").arg(util::format_metric(snapshot.psi->io_some.avg10).c_str()));
    } else {
        cpu_psi_kpi_->setText("CPU PSI: N/A");
        mem_psi_kpi_->setText("MEM PSI: N/A");
        io_psi_kpi_->setText("IO PSI: N/A");
    }

    cpu_chart_->append_sample(snapshot.cpu.total_utilization);
    mem_chart_->append_sample(snapshot.memory.utilization_percent);
}

} // namespace sysscope::ui::qt
