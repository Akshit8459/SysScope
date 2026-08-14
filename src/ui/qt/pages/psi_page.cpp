#include "sysscope/ui/qt/pages/psi_page.hpp"
#include "sysscope/util/format.hpp"
#include <QVBoxLayout>
#include <QGroupBox>

namespace sysscope::ui::qt {

PsiPage::PsiPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &PsiPage::on_snapshot_updated);
}

void PsiPage::setup_ui() {
    auto* main_layout = new QVBoxLayout(this);

    auto* cpu_group = new QGroupBox("CPU Pressure Stall Information (PSI)", this);
    auto* cpu_layout = new QVBoxLayout(cpu_group);
    cpu_some_label_ = new QLabel("CPU Some (avg10 / avg60 / avg300): 0.00% / 0.00% / 0.00%", this);
    cpu_layout->addWidget(cpu_some_label_);

    auto* mem_group = new QGroupBox("Memory Pressure Stall Information (PSI)", this);
    auto* mem_layout = new QVBoxLayout(mem_group);
    mem_some_label_ = new QLabel("Memory Some (avg10 / avg60 / avg300): 0.00% / 0.00% / 0.00%", this);
    mem_full_label_ = new QLabel("Memory Full (avg10 / avg60 / avg300): 0.00% / 0.00% / 0.00%", this);
    mem_layout->addWidget(mem_some_label_);
    mem_layout->addWidget(mem_full_label_);

    auto* io_group = new QGroupBox("Disk I/O Pressure Stall Information (PSI)", this);
    auto* io_layout = new QVBoxLayout(io_group);
    io_some_label_ = new QLabel("I/O Some (avg10 / avg60 / avg300): 0.00% / 0.00% / 0.00%", this);
    io_full_label_ = new QLabel("I/O Full (avg10 / avg60 / avg300): 0.00% / 0.00% / 0.00%", this);
    io_layout->addWidget(io_some_label_);
    io_layout->addWidget(io_full_label_);

    main_layout->addWidget(cpu_group);
    main_layout->addWidget(mem_group);
    main_layout->addWidget(io_group);
    main_layout->addStretch();
}

void PsiPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    if (!snapshot.psi.has_value()) {
        cpu_some_label_->setText("CPU Some: N/A (PSI unavailable)");
        mem_some_label_->setText("Memory Some: N/A (PSI unavailable)");
        mem_full_label_->setText("Memory Full: N/A (PSI unavailable)");
        io_some_label_->setText("I/O Some: N/A (PSI unavailable)");
        io_full_label_->setText("I/O Full: N/A (PSI unavailable)");
        return;
    }

    const auto& psi = snapshot.psi.value();
    cpu_some_label_->setText(QString("CPU Some (avg10 / avg60 / avg300): %1% / %2% / %3%")
        .arg(util::format_metric(psi.cpu_some.avg10).c_str())
        .arg(util::format_metric(psi.cpu_some.avg60).c_str())
        .arg(util::format_metric(psi.cpu_some.avg300).c_str()));

    mem_some_label_->setText(QString("Memory Some (avg10 / avg60 / avg300): %1% / %2% / %3%")
        .arg(util::format_metric(psi.memory_some.avg10).c_str())
        .arg(util::format_metric(psi.memory_some.avg60).c_str())
        .arg(util::format_metric(psi.memory_some.avg300).c_str()));

    mem_full_label_->setText(QString("Memory Full (avg10 / avg60 / avg300): %1% / %2% / %3%")
        .arg(util::format_metric(psi.memory_full.avg10).c_str())
        .arg(util::format_metric(psi.memory_full.avg60).c_str())
        .arg(util::format_metric(psi.memory_full.avg300).c_str()));

    io_some_label_->setText(QString("I/O Some (avg10 / avg60 / avg300): %1% / %2% / %3%")
        .arg(util::format_metric(psi.io_some.avg10).c_str())
        .arg(util::format_metric(psi.io_some.avg60).c_str())
        .arg(util::format_metric(psi.io_some.avg300).c_str()));

    io_full_label_->setText(QString("I/O Full (avg10 / avg60 / avg300): %1% / %2% / %3%")
        .arg(util::format_metric(psi.io_full.avg10).c_str())
        .arg(util::format_metric(psi.io_full.avg60).c_str())
        .arg(util::format_metric(psi.io_full.avg300).c_str()));
}

} // namespace sysscope::ui::qt
