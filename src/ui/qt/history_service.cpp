#include "sysscope/ui/qt/history_service.hpp"

namespace sysscope::ui::qt {

HistoryService::HistoryService(std::shared_ptr<storage::IPersistenceEngine> engine)
    : engine_(engine) {}

core::Result<HistoryResult> HistoryService::query(const HistoryQuery& q) {
    if (!engine_) {
        return core::Result<HistoryResult>::error("Persistence engine uninitialized");
    }
    auto query_res = engine_->query_history(q.metric_type, q.start_time.nanoseconds(), q.end_time.nanoseconds());
    if (!query_res.is_ok()) {
        return core::Result<HistoryResult>::error(query_res.error_message());
    }

    HistoryResult res{};
    res.records = query_res.value();
    return core::Result<HistoryResult>(res);
}

} // namespace sysscope::ui::qt
