#pragma once

#include "sysscope/storage/persistence.hpp"
#include "sysscope/core/result.hpp"
#include <memory>
#include <vector>

namespace sysscope::ui::qt {

struct HistoryQuery {
    core::Timestamp start_time;
    core::Timestamp end_time;
    std::string metric_type;
};

struct HistoryResult {
    std::vector<storage::HistoricalMetricRecord> records;
};

class HistoryService {
public:
    explicit HistoryService(std::shared_ptr<storage::IPersistenceEngine> engine);
    ~HistoryService() = default;

    core::Result<HistoryResult> query(const HistoryQuery& q);

private:
    std::shared_ptr<storage::IPersistenceEngine> engine_;
};

} // namespace sysscope::ui::qt
