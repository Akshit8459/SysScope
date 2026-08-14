#include "sysscope/platform/mock_file_system_reader.hpp"
#include <sstream>

namespace sysscope::platform {

void MockFileSystemReader::set_file_content(const std::string& path, std::string content) {
    virtual_files_[path] = std::move(content);
}

void MockFileSystemReader::add_directory_entry(const std::string& dir_path, const std::string& entry_name) {
    virtual_dirs_[dir_path].insert(entry_name);
}

void MockFileSystemReader::clear() {
    virtual_files_.clear();
    virtual_dirs_.clear();
}

core::Result<std::string> MockFileSystemReader::read_file_contents(const std::string& path) const {
    auto it = virtual_files_.find(path);
    if (it == virtual_files_.end()) {
        return core::Result<std::string>::error("Mock file not found: " + path);
    }
    return core::Result<std::string>(it->second);
}

core::Result<std::vector<std::string>> MockFileSystemReader::read_file_lines(const std::string& path) const {
    auto res = read_file_contents(path);
    if (res.is_err()) {
        return core::Result<std::vector<std::string>>::error(res.error_message());
    }
    std::vector<std::string> lines;
    std::istringstream stream(res.value());
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return core::Result<std::vector<std::string>>(lines);
}

bool MockFileSystemReader::file_exists(const std::string& path) const {
    return virtual_files_.find(path) != virtual_files_.end() || virtual_dirs_.find(path) != virtual_dirs_.end();
}

core::Result<std::vector<std::string>> MockFileSystemReader::list_directory_entries(const std::string& path) const {
    auto it = virtual_dirs_.find(path);
    if (it == virtual_dirs_.end()) {
        return core::Result<std::vector<std::string>>::error("Mock directory not found: " + path);
    }
    std::vector<std::string> entries(it->second.begin(), it->second.end());
    return core::Result<std::vector<std::string>>(entries);
}

} // namespace sysscope::platform
