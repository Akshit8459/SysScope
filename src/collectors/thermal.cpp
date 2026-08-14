#include "sysscope/collectors/thermal.hpp"
#include <sstream>

namespace sysscope::collectors {

ThermalCollector::ThermalCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::Result<core::ThermalSnapshot> ThermalCollector::collect() {
    auto tz_dir_res = fs_reader_->list_directory_entries("/sys/class/thermal");
    if (tz_dir_res.is_err()) {
        return core::Result<core::ThermalSnapshot>::error(tz_dir_res.error_message());
    }

    core::ThermalSnapshot snapshot{};

    for (const auto& entry : tz_dir_res.value()) {
        if (entry.rfind("thermal_zone", 0) != 0) continue;

        std::string type_path = "/sys/class/thermal/" + entry + "/type";
        std::string temp_path = "/sys/class/thermal/" + entry + "/temp";

        auto type_res = fs_reader_->read_file_contents(type_path);
        auto temp_res = fs_reader_->read_file_contents(temp_path);

        if (type_res.is_ok() && temp_res.is_ok()) {
            std::string zone_type = type_res.value();
            if (!zone_type.empty() && zone_type.back() == '\n') zone_type.pop_back();

            try {
                double raw_temp = std::stod(temp_res.value());
                double temp_celsius = (raw_temp > 1000.0) ? (raw_temp / 1000.0) : raw_temp;

                core::ThermalZoneInfo info{};
                info.name = entry;
                info.type = zone_type;
                info.temperature_celsius = temp_celsius;

                snapshot.zones.push_back(info);
            } catch (...) {}
        }
    }

    return core::Result<core::ThermalSnapshot>(snapshot);
}

} // namespace sysscope::collectors
