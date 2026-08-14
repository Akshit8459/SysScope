#pragma once

#include "sysscope/ui/qt/telemetry_model.hpp"
#include "sysscope/ui/qt/telemetry_bridge.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <QMainWindow>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <memory>

namespace sysscope::ui::qt {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(std::shared_ptr<platform::IFileSystemReader> fs_reader, QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_navigation_changed(int index);
    void on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot);
    void poll_telemetry();

private:
    void setup_ui();
    void setup_status_bar();

    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    TelemetryModel* model_{nullptr};
    QtTelemetryBridge* bridge_{nullptr};
    QTimer* poll_timer_{nullptr};

    QListWidget* sidebar_{nullptr};
    QStackedWidget* page_stack_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* caps_label_{nullptr};
};

} // namespace sysscope::ui::qt
