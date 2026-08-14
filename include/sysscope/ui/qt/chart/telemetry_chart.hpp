#pragma once

#include "sysscope/core/snapshot.hpp"
#include <QWidget>
#include <deque>
#include <string>
#include <vector>

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QChartView>)
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#endif

namespace sysscope::ui::qt {

class TelemetryChart : public QWidget {
    Q_OBJECT

public:
    explicit TelemetryChart(const std::string& title, const std::string& y_label, double max_y = 100.0, QWidget* parent = nullptr);
    ~TelemetryChart() override = default;

    void update_series(const std::vector<double>& values);
    void append_sample(double value);

private:
    void setup_chart(const std::string& title, const std::string& y_label, double max_y);

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QChartView>)
    QChart* chart_{nullptr};
    QChartView* chart_view_{nullptr};
    QLineSeries* series_{nullptr};
    QValueAxis* axis_x_{nullptr};
    QValueAxis* axis_y_{nullptr};
#endif
    std::deque<double> buffer_;
    static constexpr size_t MAX_POINTS = 60; // 60 seconds of history
};

} // namespace sysscope::ui::qt
