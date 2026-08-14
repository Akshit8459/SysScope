#pragma once

#include "sysscope/core/types.hpp"
#include "sysscope/core/snapshot.hpp"
#include <string>
#include <vector>

namespace sysscope::analytics {

struct Evidence {
    core::MetricType metric;
    double observed_value{0.0};
    double threshold{0.0};
    double contribution{0.0};
    std::string explanation;
};

struct Diagnosis {
    std::string condition;
    double confidence{0.0};
    std::vector<Evidence> evidence;
};

class CorrelationEngine {
public:
    CorrelationEngine() = default;
    ~CorrelationEngine() = default;

    [[nodiscard]] std::vector<Diagnosis> analyze(const core::MetricSnapshot& snapshot) const;
};

} // namespace sysscope::analytics
