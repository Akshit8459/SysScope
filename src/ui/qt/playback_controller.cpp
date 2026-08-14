#include "sysscope/ui/qt/playback_controller.hpp"

namespace sysscope::ui::qt {

PlaybackController::PlaybackController(QObject* parent)
    : QObject(parent) {
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &PlaybackController::tick);
}

void PlaybackController::load_records(const std::vector<storage::HistoricalMetricRecord>& records) {
    records_ = records;
    current_index_ = 0;
}

void PlaybackController::start() {
    if (records_.empty()) return;
    timer_->start(static_cast<int>(100 / speed_));
}

void PlaybackController::pause() {
    timer_->stop();
}

void PlaybackController::stop() {
    timer_->stop();
    current_index_ = 0;
}

void PlaybackController::set_speed(double multiplier) {
    speed_ = multiplier;
    if (timer_->isActive()) {
        timer_->setInterval(static_cast<int>(100 / speed_));
    }
}

void PlaybackController::tick() {
    if (current_index_ >= records_.size()) {
        timer_->stop();
        emit playback_finished();
        return;
    }
    emit playback_record(records_[current_index_++]);
}

} // namespace sysscope::ui::qt
