#include "sysscope/ui/qt/main_window.hpp"
#include "sysscope/platform/real_file_system_reader.hpp"
#include <QApplication>
#include <memory>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    auto fs_reader = std::make_shared<sysscope::platform::RealFileSystemReader>();
    sysscope::ui::qt::MainWindow main_window(fs_reader);
    main_window.show();

    return app.exec();
}
