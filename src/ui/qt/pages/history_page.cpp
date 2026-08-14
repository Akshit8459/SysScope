#include "sysscope/ui/qt/pages/history_page.hpp"
#include "sysscope/storage/sqlite_persistence.hpp"
#include "sysscope/platform/real_file_system_reader.hpp"
#include "sysscope/util/format.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>

namespace sysscope::ui::qt {

HistoryPage::HistoryPage(QWidget* parent)
    : QWidget(parent) {
    auto fs_reader = std::make_shared<platform::RealFileSystemReader>();
    auto sqlite_engine = std::make_shared<storage::SqlitePersistenceEngine>(fs_reader);
    sqlite_engine->initialize("/tmp/sysscope_history.db");

    history_service_ = std::make_shared<HistoryService>(sqlite_engine);
    playback_ = new PlaybackController(this);

    setup_ui();

    connect(query_btn_, &QPushButton::clicked, this, &HistoryPage::on_query_clicked);
    connect(play_btn_, &QPushButton::clicked, this, &HistoryPage::on_play_clicked);
    connect(pause_btn_, &QPushButton::clicked, this, &HistoryPage::on_pause_clicked);
    connect(stop_btn_, &QPushButton::clicked, this, &HistoryPage::on_stop_clicked);
    connect(playback_, &PlaybackController::playback_record, this, &HistoryPage::on_playback_record);
}

void HistoryPage::setup_ui() {
    auto* main_layout = new QVBoxLayout(this);

    // Controls Header
    auto* ctrl_group = new QGroupBox("SQLite Historical Time-Series Query & Playback Controls", this);
    auto* ctrl_layout = new QHBoxLayout(ctrl_group);

    metric_selector_ = new QComboBox(this);
    metric_selector_->addItem("cpu_utilization");
    metric_selector_->addItem("memory_utilization");
    metric_selector_->addItem("cpu_psi_avg10");
    metric_selector_->addItem("io_psi_avg10");

    query_btn_ = new QPushButton("Execute Query", this);
    play_btn_ = new QPushButton("▶ Play", this);
    pause_btn_ = new QPushButton("⏸ Pause", this);
    stop_btn_ = new QPushButton("⏹ Stop", this);

    ctrl_layout->addWidget(new QLabel("Metric:", this));
    ctrl_layout->addWidget(metric_selector_);
    ctrl_layout->addWidget(query_btn_);
    ctrl_layout->addWidget(play_btn_);
    ctrl_layout->addWidget(pause_btn_);
    ctrl_layout->addWidget(stop_btn_);
    ctrl_layout->addStretch();

    main_layout->addWidget(ctrl_group);

    // Chart & Table Split View
    history_chart_ = new TelemetryChart("Historical Metric Replay Timeline", "Value", 100.0, this);
    history_table_ = new QTableWidget(0, 4, this);
    history_table_->setHorizontalHeaderLabels({"Timestamp", "Metric", "Entity", "Value"});
    history_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    main_layout->addWidget(history_chart_);
    main_layout->addWidget(history_table_);
}

void HistoryPage::on_query_clicked() {
    HistoryQuery q{};
    q.start_time = core::Timestamp(core::Timestamp::now().nanoseconds() - 3600000000000ULL);
    q.end_time = core::Timestamp::now();
    q.metric_type = metric_selector_->currentText().toStdString();

    auto res = history_service_->query(q);
    if (res.is_ok()) {
        current_records_ = res.value().records;
        playback_->load_records(current_records_);

        history_table_->setRowCount(0);
        std::vector<double> vals;
        for (const auto& rec : current_records_) {
            vals.push_back(rec.value);
            int row = history_table_->rowCount();
            history_table_->insertRow(row);
            history_table_->setItem(row, 0, new QTableWidgetItem(core::Timestamp(rec.timestamp_ns).to_iso_string().c_str()));
            history_table_->setItem(row, 1, new QTableWidgetItem(rec.metric_type.c_str()));
            history_table_->setItem(row, 2, new QTableWidgetItem(rec.entity.c_str()));
            history_table_->setItem(row, 3, new QTableWidgetItem(util::format_metric(rec.value).c_str()));
        }
        history_chart_->update_series(vals);
    }
}

void HistoryPage::on_play_clicked() {
    playback_->start();
}

void HistoryPage::on_pause_clicked() {
    playback_->pause();
}

void HistoryPage::on_stop_clicked() {
    playback_->stop();
}

void HistoryPage::on_playback_record(const sysscope::storage::HistoricalMetricRecord& record) {
    history_chart_->append_sample(record.value);
}

} // namespace sysscope::ui::qt
