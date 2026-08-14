#pragma once

#include "sysscope/ui/qt/history_service.hpp"
#include "sysscope/ui/qt/playback_controller.hpp"
#include "sysscope/ui/qt/chart/telemetry_chart.hpp"
#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>

namespace sysscope::ui::qt {

class HistoryPage : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPage(QWidget* parent = nullptr);
    ~HistoryPage() override = default;

private slots:
    void on_query_clicked();
    void on_play_clicked();
    void on_pause_clicked();
    void on_stop_clicked();
    void on_playback_record(const sysscope::storage::HistoricalMetricRecord& record);

private:
    void setup_ui();

    std::shared_ptr<HistoryService> history_service_;
    PlaybackController* playback_{nullptr};

    QComboBox* metric_selector_{nullptr};
    QPushButton* query_btn_{nullptr};
    QPushButton* play_btn_{nullptr};
    QPushButton* pause_btn_{nullptr};
    QPushButton* stop_btn_{nullptr};
    QTableWidget* history_table_{nullptr};
    TelemetryChart* history_chart_{nullptr};

    std::vector<storage::HistoricalMetricRecord> current_records_;
};

} // namespace sysscope::ui::qt
