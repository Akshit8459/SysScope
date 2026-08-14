#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>

namespace sysscope::collectors {

class ThermalCollector {
public:
    explicit ThermalCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~ThermalCollector() = default;

    [[nodiscard]] core::Result<core::ThermalSnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
};

} // namespace sysscope::collectors
