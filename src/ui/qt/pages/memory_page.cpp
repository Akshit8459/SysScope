#include "sysscope/ui/qt/pages/memory_page.hpp"
#include "sysscope/util/format.hpp"
#include <QVBoxLayout>
#include <QGroupBox>

namespace sysscope::ui::qt {

MemoryPage::MemoryPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &MemoryPage::on_snapshot_updated);
}

void MemoryPage::setup_ui() {
    auto* main_layout = new QVBoxLayout(this);

    auto* mem_group = new QGroupBox("Linux System Memory Breakdown (MemAvailable Priority)", this);
    auto* mem_layout = new QVBoxLayout(mem_group);

    total_ram_label_ = new QLabel("Total Physical RAM: 0 B", this);
    avail_ram_label_ = new QLabel("Available Memory (MemAvailable): 0 B", this);
    used_ram_label_ = new QLabel("Used Memory: 0 B", this);
    cached_ram_label_ = new QLabel("Buffers / Page Cache: 0 B", this);
    swap_label_ = new QLabel("Swap Space: 0 B", this);

    ram_bar_ = new QProgressBar(this);
    ram_bar_->setRange(0, 100);
    ram_bar_->setValue(0);

    mem_layout->addWidget(total_ram_label_);
    mem_layout->addWidget(avail_ram_label_);
    mem_layout->addWidget(used_ram_label_);
    mem_layout->addWidget(cached_ram_label_);
    mem_layout->addWidget(swap_label_);
    mem_layout->addWidget(ram_bar_);

    main_layout->addWidget(mem_group);
    main_layout->addStretch();
}

void MemoryPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    total_ram_label_->setText(QString("Total Physical RAM: %1").arg(util::format_bytes(snapshot.memory.total_bytes).c_str()));
    avail_ram_label_->setText(QString("Available Memory (MemAvailable): %1").arg(util::format_bytes(snapshot.memory.available_bytes).c_str()));
    used_ram_label_->setText(QString("Used Memory: %1").arg(util::format_bytes(snapshot.memory.used_bytes).c_str()));
    cached_ram_label_->setText(QString("Buffers / Page Cache: %1").arg(util::format_bytes(snapshot.memory.cached_bytes).c_str()));
    swap_label_->setText(QString("Swap Space: %1").arg(util::format_bytes(snapshot.memory.swap_used_bytes).c_str()));

    ram_bar_->setValue(static_cast<int>(snapshot.memory.utilization_percent));
}

} // namespace sysscope::ui::qt
