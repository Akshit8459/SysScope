#pragma once

#include "sysscope/storage/persistence.hpp"
#include <QObject>
#include <QTimer>
#include <vector>

namespace sysscope::ui::qt {

class PlaybackController : public QObject {
    Q_OBJECT

public:
    explicit PlaybackController(QObject* parent = nullptr);
    ~PlaybackController() override = default;

    void load_records(const std::vector<storage::HistoricalMetricRecord>& records);
    void start();
    void pause();
    void stop();
    void set_speed(double multiplier);

signals:
    void playback_record(const sysscope::storage::HistoricalMetricRecord& record);
    void playback_finished();

private slots:
    void tick();

private:
    std::vector<storage::HistoricalMetricRecord> records_;
    size_t current_index_{0};
    double speed_{1.0};
    QTimer* timer_{nullptr};
};

} // namespace sysscope::ui::qt
