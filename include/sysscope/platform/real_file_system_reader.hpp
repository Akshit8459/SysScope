#pragma once

#include "sysscope/platform/file_system_reader.hpp"

namespace sysscope::platform {

class RealFileSystemReader : public IFileSystemReader {
public:
    RealFileSystemReader() = default;
    ~RealFileSystemReader() override = default;

    [[nodiscard]] core::Result<std::string> read_file_contents(const std::string& path) const override;
    [[nodiscard]] core::Result<std::vector<std::string>> read_file_lines(const std::string& path) const override;
    [[nodiscard]] bool file_exists(const std::string& path) const override;
    [[nodiscard]] core::Result<std::vector<std::string>> list_directory_entries(const std::string& path) const override;
};

} // namespace sysscope::platform
