#pragma once

#include "sysscope/core/timestamp.hpp"
#include "sysscope/analytics/correlation.hpp"
#include <QObject>
#include <QMetaType>
#include <string>
#include <vector>

namespace sysscope::ui::qt {

enum class DiagnosisType {
    CpuSchedulingContention,
    MemoryPressure,
    IoBottleneck,
    ThermalThrottling
};

enum class Severity {
    Low,
    Medium,
    High,
    Critical
};

struct DiagnosisEvent {
    core::Timestamp start_time;
    core::Timestamp end_time;
    DiagnosisType type{DiagnosisType::CpuSchedulingContention};
    Severity severity{Severity::Medium};
    std::string condition;
    std::vector<std::string> triggered_rules;
    std::vector<analytics::Evidence> evidence;
};

class DiagnosisModel : public QObject {
    Q_OBJECT

public:
    explicit DiagnosisModel(QObject* parent = nullptr);
    ~DiagnosisModel() override = default;

    const std::vector<DiagnosisEvent>& active_incidents() const;
    const std::vector<DiagnosisEvent>& historical_incidents() const;

public slots:
    void add_incident(const sysscope::ui::qt::DiagnosisEvent& event);

signals:
    void incident_raised(const sysscope::ui::qt::DiagnosisEvent& event);

private:
    std::vector<DiagnosisEvent> active_incidents_;
    std::vector<DiagnosisEvent> historical_incidents_;
};

} // namespace sysscope::ui::qt

Q_DECLARE_METATYPE(sysscope::ui::qt::DiagnosisEvent)
