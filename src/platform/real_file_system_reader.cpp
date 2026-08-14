#include "sysscope/platform/real_file_system_reader.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

namespace sysscope::platform {

namespace fs = std::filesystem;

core::Result<std::string> RealFileSystemReader::read_file_contents(const std::string& path) const {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return core::Result<std::string>::error("Failed to open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return core::Result<std::string>(ss.str());
}

core::Result<std::vector<std::string>> RealFileSystemReader::read_file_lines(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        return core::Result<std::vector<std::string>>::error("Failed to open file: " + path);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return core::Result<std::vector<std::string>>(lines);
}

bool RealFileSystemReader::file_exists(const std::string& path) const {
    std::error_code ec;
    return fs::exists(path, ec);
}

core::Result<std::vector<std::string>> RealFileSystemReader::list_directory_entries(const std::string& path) const {
    std::error_code ec;
    if (!fs::exists(path, ec) || !fs::is_directory(path, ec)) {
        return core::Result<std::vector<std::string>>::error("Directory does not exist: " + path);
    }
    std::vector<std::string> entries;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        entries.push_back(entry.path().filename().string());
    }
    if (ec) {
        return core::Result<std::vector<std::string>>::error("Error iterating directory: " + ec.message());
    }
    return core::Result<std::vector<std::string>>(entries);
}

} // namespace sysscope::platform
