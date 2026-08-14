#pragma once

#include "sysscope/storage/persistence.hpp"
#include "sysscope/platform/file_system_reader.hpp"
#include <fstream>
#include <mutex>

namespace sysscope::storage {

class SqlitePersistenceEngine : public IPersistenceEngine {
public:
    explicit SqlitePersistenceEngine(std::shared_ptr<platform::IFileSystemReader> fs_reader);
    ~SqlitePersistenceEngine() override;

    bool initialize(const std::string& db_path) override;
    core::Result<bool> store_snapshot(const core::MetricSnapshot& snapshot) override;
    core::Result<bool> store_diagnosis(const analytics::Diagnosis& diagnosis, uint64_t timestamp_ns) override;
    core::Result<std::vector<HistoricalMetricRecord>> query_history(const std::string& metric_type, uint64_t start_ns, uint64_t end_ns) override;
    core::Result<std::vector<HistoricalDiagnosisRecord>> query_diagnoses(uint64_t start_ns, uint64_t end_ns) override;

private:
    std::shared_ptr<platform::IFileSystemReader> fs_reader_;
    std::string db_path_;
    bool initialized_{false};
    std::mutex db_mutex_;
    std::vector<HistoricalMetricRecord> metric_store_;
    std::vector<HistoricalDiagnosisRecord> diagnosis_store_;
};

} // namespace sysscope::storage
