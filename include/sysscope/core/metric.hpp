#pragma once

#include "sysscope/core/timestamp.hpp"
#include "sysscope/core/types.hpp"
#include <string>

namespace sysscope::core {

struct MetricSample {
    Timestamp timestamp;
    MetricType type;
    std::string entity; // e.g. "CPU_AGGREGATE", "CPU0", "GPU0", "eth0", "PID1821"
    double value{0.0};
    Unit unit{Unit::Percent};
    Source source{Source::Procfs};
};

} // namespace sysscope::core
