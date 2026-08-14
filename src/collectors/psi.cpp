#include "sysscope/collectors/psi.hpp"
#include <sstream>

namespace sysscope::collectors {

PsiCollector::PsiCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

core::PsiPressureValues PsiCollector::parse_pressure_line(const std::string& line) {
    core::PsiPressureValues vals{};
    std::istringstream ss(line);
    std::string prefix;
    ss >> prefix;

    std::string token;
    while (ss >> token) {
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;

        std::string key = token.substr(0, eq);
        std::string val_str = token.substr(eq + 1);

        try {
            if (key == "avg10") vals.avg10 = std::stod(val_str);
            else if (key == "avg60") vals.avg60 = std::stod(val_str);
            else if (key == "avg300") vals.avg300 = std::stod(val_str);
            else if (key == "total") vals.total_microseconds = std::stoull(val_str);
        } catch (...) {}
    }
    return vals;
}

core::Result<core::PsiSnapshot> PsiCollector::collect() {
    core::PsiSnapshot snapshot{};

    // Read CPU Pressure (/proc/pressure/cpu)
    auto cpu_res = fs_reader_->read_file_lines("/proc/pressure/cpu");
    if (cpu_res.is_ok()) {
        for (const auto& line : cpu_res.value()) {
            if (line.rfind("some", 0) == 0) {
                snapshot.cpu_some = parse_pressure_line(line);
            }
        }
    }

    // Read Memory Pressure (/proc/pressure/memory)
    auto mem_res = fs_reader_->read_file_lines("/proc/pressure/memory");
    if (mem_res.is_ok()) {
        for (const auto& line : mem_res.value()) {
            if (line.rfind("some", 0) == 0) {
                snapshot.memory_some = parse_pressure_line(line);
            } else if (line.rfind("full", 0) == 0) {
                snapshot.memory_full = parse_pressure_line(line);
            }
        }
    }

    // Read I/O Pressure (/proc/pressure/io)
    auto io_res = fs_reader_->read_file_lines("/proc/pressure/io");
    if (io_res.is_ok()) {
        for (const auto& line : io_res.value()) {
            if (line.rfind("some", 0) == 0) {
                snapshot.io_some = parse_pressure_line(line);
            } else if (line.rfind("full", 0) == 0) {
                snapshot.io_full = parse_pressure_line(line);
            }
        }
    }

    return core::Result<core::PsiSnapshot>(snapshot);
}

} // namespace sysscope::collectors
