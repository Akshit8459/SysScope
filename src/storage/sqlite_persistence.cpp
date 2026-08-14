#include "sysscope/storage/sqlite_persistence.hpp"
#include "sysscope/util/format.hpp"
#include <sstream>
#include <algorithm>

namespace sysscope::storage {

SqlitePersistenceEngine::SqlitePersistenceEngine(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {}

SqlitePersistenceEngine::~SqlitePersistenceEngine() = default;

bool SqlitePersistenceEngine::initialize(const std::string& db_path) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    db_path_ = db_path;
    initialized_ = true;
    return true;
}

core::Result<bool> SqlitePersistenceEngine::store_snapshot(const core::MetricSnapshot& snapshot) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!initialized_) return core::Result<bool>::error("SqlitePersistenceEngine uninitialized.");

    HistoricalMetricRecord rec{};
    rec.timestamp_ns = snapshot.timestamp.nanoseconds();
    rec.metric_type = "cpu_utilization";
    rec.entity = "system";
    rec.value = snapshot.cpu.total_utilization;
    rec.unit = "%";
    rec.source = "procfs";
    metric_store_.push_back(rec);

    return core::Result<bool>(true);
}

core::Result<bool> SqlitePersistenceEngine::store_diagnosis(const analytics::Diagnosis& diagnosis, uint64_t timestamp_ns) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    if (!initialized_) return core::Result<bool>::error("SqlitePersistenceEngine uninitialized.");

    HistoricalDiagnosisRecord rec{};
    rec.timestamp_ns = timestamp_ns;
    rec.condition = diagnosis.condition;
    rec.confidence = diagnosis.confidence;
    if (!diagnosis.evidence.empty()) {
        rec.evidence_summary = diagnosis.evidence[0].explanation;
    }
    diagnosis_store_.push_back(rec);

    return core::Result<bool>(true);
}

core::Result<std::vector<HistoricalMetricRecord>> SqlitePersistenceEngine::query_history(const std::string& metric_type, uint64_t start_ns, uint64_t end_ns) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    (void)metric_type;
    std::vector<HistoricalMetricRecord> result;
    for (const auto& rec : metric_store_) {
        if (rec.timestamp_ns >= start_ns && rec.timestamp_ns <= end_ns) {
            result.push_back(rec);
        }
    }
    return core::Result<std::vector<HistoricalMetricRecord>>(result);
}

core::Result<std::vector<HistoricalDiagnosisRecord>> SqlitePersistenceEngine::query_diagnoses(uint64_t start_ns, uint64_t end_ns) {
    std::lock_guard<std::mutex> lock(db_mutex_);
    std::vector<HistoricalDiagnosisRecord> result;
    for (const auto& rec : diagnosis_store_) {
        if (rec.timestamp_ns >= start_ns && rec.timestamp_ns <= end_ns) {
            result.push_back(rec);
        }
    }
    return core::Result<std::vector<HistoricalDiagnosisRecord>>(result);
}

} // namespace sysscope::storage
