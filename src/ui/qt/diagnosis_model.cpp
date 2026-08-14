#include "sysscope/ui/qt/diagnosis_model.hpp"

namespace sysscope::ui::qt {

DiagnosisModel::DiagnosisModel(QObject* parent)
    : QObject(parent) {}

const std::vector<DiagnosisEvent>& DiagnosisModel::active_incidents() const {
    return active_incidents_;
}

const std::vector<DiagnosisEvent>& DiagnosisModel::historical_incidents() const {
    return historical_incidents_;
}

void DiagnosisModel::add_incident(const DiagnosisEvent& event) {
    active_incidents_.push_back(event);
    historical_incidents_.push_back(event);
    emit incident_raised(event);
}

} // namespace sysscope::ui::qt
