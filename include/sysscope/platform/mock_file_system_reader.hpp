#pragma once

#include "sysscope/platform/file_system_reader.hpp"
#include <unordered_map>
#include <unordered_set>

namespace sysscope::platform {

class MockFileSystemReader : public IFileSystemReader {
public:
    MockFileSystemReader() = default;
    ~MockFileSystemReader() override = default;

    void set_file_content(const std::string& path, std::string content);
    void add_directory_entry(const std::string& dir_path, const std::string& entry_name);
    void clear();

    [[nodiscard]] core::Result<std::string> read_file_contents(const std::string& path) const override;
    [[nodiscard]] core::Result<std::vector<std::string>> read_file_lines(const std::string& path) const override;
    [[nodiscard]] bool file_exists(const std::string& path) const override;
    [[nodiscard]] core::Result<std::vector<std::string>> list_directory_entries(const std::string& path) const override;

private:
    std::unordered_map<std::string, std::string> virtual_files_;
    std::unordered_map<std::string, std::unordered_set<std::string>> virtual_dirs_;
};

} // namespace sysscope::platform
