#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>

namespace sysscope::collectors {

class PsiCollector {
public:
    explicit PsiCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~PsiCollector() = default;

    [[nodiscard]] core::Result<core::PsiSnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    static core::PsiPressureValues parse_pressure_line(const std::string& line);
};

} // namespace sysscope::collectors
