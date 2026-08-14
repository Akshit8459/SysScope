#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include "sysscope/analytics/correlation.hpp"
#include <string>
#include <vector>
#include <memory>

namespace sysscope::storage {

struct HistoricalMetricRecord {
    uint64_t timestamp_ns{0};
    std::string metric_type;
    std::string entity;
    double value{0.0};
    std::string unit;
    std::string source;
};

struct HistoricalDiagnosisRecord {
    uint64_t timestamp_ns{0};
    std::string condition;
    double confidence{0.0};
    std::string evidence_summary;
};

class IPersistenceEngine {
public:
    virtual ~IPersistenceEngine() = default;

    virtual bool initialize(const std::string& db_path) = 0;
    virtual core::Result<bool> store_snapshot(const core::MetricSnapshot& snapshot) = 0;
    virtual core::Result<bool> store_diagnosis(const analytics::Diagnosis& diagnosis, uint64_t timestamp_ns) = 0;
    virtual core::Result<std::vector<HistoricalMetricRecord>> query_history(const std::string& metric_type, uint64_t start_ns, uint64_t end_ns) = 0;
    virtual core::Result<std::vector<HistoricalDiagnosisRecord>> query_diagnoses(uint64_t start_ns, uint64_t end_ns) = 0;
};

class MockPersistenceEngine : public IPersistenceEngine {
public:
    MockPersistenceEngine() = default;
    ~MockPersistenceEngine() override = default;

    bool initialize(const std::string& db_path) override {
        (void)db_path;
        initialized_ = true;
        return true;
    }

    core::Result<bool> store_snapshot(const core::MetricSnapshot& snapshot) override {
        if (!initialized_) return core::Result<bool>::error("Persistence engine uninitialized.");
        HistoricalMetricRecord rec{};
        rec.timestamp_ns = snapshot.timestamp.nanoseconds();
        rec.metric_type = "cpu_utilization";
        rec.entity = "system";
        rec.value = snapshot.cpu.total_utilization;
        rec.unit = "%";
        rec.source = "procfs";
        metric_records_.push_back(rec);
        return core::Result<bool>(true);
    }

    core::Result<bool> store_diagnosis(const analytics::Diagnosis& diagnosis, uint64_t timestamp_ns) override {
        if (!initialized_) return core::Result<bool>::error("Persistence engine uninitialized.");
        HistoricalDiagnosisRecord rec{};
        rec.timestamp_ns = timestamp_ns;
        rec.condition = diagnosis.condition;
        rec.confidence = diagnosis.confidence;
        if (!diagnosis.evidence.empty()) {
            rec.evidence_summary = diagnosis.evidence[0].explanation;
        }
        diagnosis_records_.push_back(rec);
        return core::Result<bool>(true);
    }

    core::Result<std::vector<HistoricalMetricRecord>> query_history(const std::string& metric_type, uint64_t start_ns, uint64_t end_ns) override {
        (void)metric_type; (void)start_ns; (void)end_ns;
        return core::Result<std::vector<HistoricalMetricRecord>>(metric_records_);
    }

    core::Result<std::vector<HistoricalDiagnosisRecord>> query_diagnoses(uint64_t start_ns, uint64_t end_ns) override {
        (void)start_ns; (void)end_ns;
        return core::Result<std::vector<HistoricalDiagnosisRecord>>(diagnosis_records_);
    }

private:
    bool initialized_{false};
    std::vector<HistoricalMetricRecord> metric_records_;
    std::vector<HistoricalDiagnosisRecord> diagnosis_records_;
};

} // namespace sysscope::storage
