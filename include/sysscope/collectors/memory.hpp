#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <memory>

namespace sysscope::collectors {

class MemoryCollector {
public:
    explicit MemoryCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~MemoryCollector() = default;

    [[nodiscard]] core::Result<core::MemorySnapshot> collect();

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
};

} // namespace sysscope::collectors
