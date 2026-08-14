#include "sysscope/ui/qt/chart/telemetry_chart.hpp"
#include <QVBoxLayout>

namespace sysscope::ui::qt {

TelemetryChart::TelemetryChart(const std::string& title, const std::string& y_label, double max_y, QWidget* parent)
    : QWidget(parent) {
    setup_chart(title, y_label, max_y);
}

void TelemetryChart::setup_chart(const std::string& title, const std::string& y_label, double max_y) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QChartView>)
    chart_ = new QChart();
    chart_->setTitle(QString::fromStdString(title));
    chart_->legend()->hide();

    series_ = new QLineSeries();
    chart_->addSeries(series_);

    axis_x_ = new QValueAxis();
    axis_x_->setRange(0, MAX_POINTS);
    axis_x_->setLabelFormat("%d s");
    chart_->addAxis(axis_x_, Qt::AlignBottom);
    series_->attachAxis(axis_x_);

    axis_y_ = new QValueAxis();
    axis_y_->setRange(0, max_y);
    axis_y_->setLabelFormat("%.1f");
    axis_y_->setTitleText(QString::fromStdString(y_label));
    chart_->addAxis(axis_y_, Qt::AlignLeft);
    series_->attachAxis(axis_y_);

    chart_view_ = new QChartView(chart_, this);
    chart_view_->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chart_view_);
#endif
}

void TelemetryChart::update_series(const std::vector<double>& values) {
    buffer_.clear();
    for (double val : values) {
        buffer_.push_back(val);
    }
    while (buffer_.size() > MAX_POINTS) {
        buffer_.pop_front();
    }

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QChartView>)
    if (!series_) return;
    QList<QPointF> points;
    int idx = 0;
    for (double val : buffer_) {
        points.append(QPointF(idx++, val));
    }
    series_->replace(points);
#endif
}

void TelemetryChart::append_sample(double value) {
    buffer_.push_back(value);
    if (buffer_.size() > MAX_POINTS) {
        buffer_.pop_front();
    }

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QChartView>)
    if (!series_) return;
    QList<QPointF> points;
    int idx = 0;
    for (double val : buffer_) {
        points.append(QPointF(idx++, val));
    }
    series_->replace(points);
#endif
}

} // namespace sysscope::ui::qt
