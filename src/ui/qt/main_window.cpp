#include "sysscope/ui/qt/main_window.hpp"
#include "sysscope/core/version.hpp"
#include "sysscope/platform/platform.hpp"
#include "sysscope/collectors/cpu.hpp"
#include "sysscope/collectors/memory.hpp"
#include "sysscope/util/format.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QHeaderView>

#include "sysscope/ui/qt/pages/dashboard_page.hpp"
#include "sysscope/ui/qt/pages/cpu_page.hpp"
#include "sysscope/ui/qt/pages/memory_page.hpp"
#include "sysscope/ui/qt/pages/psi_page.hpp"
#include "sysscope/ui/qt/pages/process_page.hpp"
#include "sysscope/ui/qt/pages/diagnostics_page.hpp"
#include "sysscope/ui/qt/pages/history_page.hpp"

namespace sysscope::ui::qt {

MainWindow::MainWindow(std::shared_ptr<platform::IFileSystemReader> fs_reader, QWidget* parent)
    : QMainWindow(parent), fs_reader_(fs_reader) {
    model_ = new TelemetryModel(this);
    bridge_ = new QtTelemetryBridge(model_, this);

    setup_ui();
    setup_status_bar();

    connect(sidebar_, &QListWidget::currentRowChanged, this, &MainWindow::on_navigation_changed);
    connect(model_, &TelemetryModel::snapshot_updated, this, &MainWindow::on_snapshot_updated);

    poll_timer_ = new QTimer(this);
    connect(poll_timer_, &QTimer::timeout, this, &MainWindow::poll_telemetry);
    poll_timer_->start(250); // 250 ms GUI refresh rate
}

MainWindow::~MainWindow() = default;

void MainWindow::setup_ui() {
    setWindowTitle(QString("SysScope v%1 — Qt Desktop Telemetry Visualizer").arg(SYSSCOPE_VERSION_STRING));
    resize(1280, 800);

    auto* central_widget = new QWidget(this);
    auto* main_layout = new QHBoxLayout(central_widget);
    main_layout->setContentsMargins(0, 0, 0, 0);

    sidebar_ = new QListWidget(this);
    sidebar_->setFixedWidth(220);
    sidebar_->addItem("Overview Dashboard");
    sidebar_->addItem("CPU Cores Grid");
    sidebar_->addItem("Memory & Disk");
    sidebar_->addItem("PSI Pressure Stalls");
    sidebar_->addItem("Process Explorer");
    sidebar_->addItem("Diagnostics & Incidents");
    sidebar_->addItem("SQLite History");
    sidebar_->setCurrentRow(0);

    page_stack_ = new QStackedWidget(this);
    page_stack_->addWidget(new DashboardPage(model_, this));
    page_stack_->addWidget(new CpuPage(model_, this));
    page_stack_->addWidget(new MemoryPage(model_, this));
    page_stack_->addWidget(new PsiPage(model_, this));
    page_stack_->addWidget(new ProcessPage(model_, this));
    page_stack_->addWidget(new DiagnosticsPage(model_, this));
    page_stack_->addWidget(new HistoryPage(this));

    main_layout->addWidget(sidebar_);
    main_layout->addWidget(page_stack_);
    setCentralWidget(central_widget);
}

void MainWindow::setup_status_bar() {
    status_label_ = new QLabel("System Target: " + QString::fromStdString(platform::get_platform_name()), this);
    caps_label_ = new QLabel("Capabilities: Thermal [✗] | GPU [✗] | NPU [✗] | DSP [✗] | Net [✓] | Disk [✓]", this);

    statusBar()->addWidget(status_label_, 1);
    statusBar()->addPermanentWidget(caps_label_);
}

void MainWindow::on_navigation_changed(int index) {
    if (index >= 0 && index < page_stack_->count()) {
        page_stack_->setCurrentIndex(index);
    }
}

void MainWindow::poll_telemetry() {
    collectors::CpuCollector cpu_col(fs_reader_);
    collectors::MemoryCollector mem_col(fs_reader_);

    core::MetricSnapshot snap{};
    snap.timestamp = core::Timestamp::now();
    
    auto cpu_res = cpu_col.collect();
    if (cpu_res.is_ok()) snap.cpu = cpu_res.value();

    auto mem_res = mem_col.collect();
    if (mem_res.is_ok()) snap.memory = mem_res.value();

    bridge_->post_snapshot(snap);
}

void MainWindow::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    QString status = QString("Target OS: %1 | CPU: %2% | RAM: %3% | Queue Drops: 0 | Sampling: 100ms")
        .arg(QString::fromStdString(platform::get_platform_name()))
        .arg(util::format_metric(snapshot.cpu.total_utilization).c_str())
        .arg(util::format_metric(snapshot.memory.utilization_percent).c_str());
    status_label_->setText(status);
}

} // namespace sysscope::ui::qt
