#pragma once

#include "sysscope/core/result.hpp"
#include <string>
#include <vector>

namespace sysscope::platform {

class IFileSystemReader {
public:
    virtual ~IFileSystemReader() = default;

    [[nodiscard]] virtual core::Result<std::string> read_file_contents(const std::string& path) const = 0;
    [[nodiscard]] virtual core::Result<std::vector<std::string>> read_file_lines(const std::string& path) const = 0;
    [[nodiscard]] virtual bool file_exists(const std::string& path) const = 0;
    [[nodiscard]] virtual core::Result<std::vector<std::string>> list_directory_entries(const std::string& path) const = 0;
};

} // namespace sysscope::platform
