#include "sysscope/ui/qt/pages/diagnostics_page.hpp"
#include "sysscope/analytics/correlation.hpp"
#include "sysscope/util/format.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace sysscope::ui::qt {

DiagnosticsPage::DiagnosticsPage(TelemetryModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    diagnosis_model_ = new DiagnosisModel(this);
    setup_ui();
    connect(model_, &TelemetryModel::snapshot_updated, this, &DiagnosticsPage::on_snapshot_updated);
    connect(incident_list_, &QListWidget::currentRowChanged, this, &DiagnosticsPage::on_incident_selected);
}

void DiagnosticsPage::setup_ui() {
    auto* main_layout = new QHBoxLayout(this);

    auto* list_group = new QGroupBox("Detected Diagnostic Incidents & Alerts", this);
    auto* list_layout = new QVBoxLayout(list_group);

    incident_list_ = new QListWidget(this);
    list_layout->addWidget(incident_list_);

    auto* detail_group = new QGroupBox("Incident Inspector & Evidence Panel", this);
    detail_group->setFixedWidth(360);
    auto* detail_layout = new QVBoxLayout(detail_group);

    detail_condition_ = new QLabel("Condition: No Incident Selected", this);
    detail_severity_ = new QLabel("Severity: NORMAL", this);
    detail_duration_ = new QLabel("Duration: 0.0 s", this);
    detail_rules_ = new QLabel("Triggered Rules:\n- None", this);
    detail_evidence_ = new QLabel("Evidence Summary:\n- System operating normally.", this);

    detail_layout->addWidget(detail_condition_);
    detail_layout->addWidget(detail_severity_);
    detail_layout->addWidget(detail_duration_);
    detail_layout->addWidget(detail_rules_);
    detail_layout->addWidget(detail_evidence_);
    detail_layout->addStretch();

    main_layout->addWidget(list_group, 1);
    main_layout->addWidget(detail_group);
}

void DiagnosticsPage::on_snapshot_updated(const sysscope::core::MetricSnapshot& snapshot) {
    analytics::CorrelationEngine engine;
    auto diagnoses = engine.analyze(snapshot);
    for (const auto& diag : diagnoses) {
        DiagnosisEvent event{};
        event.start_time = snapshot.timestamp;
        event.end_time = snapshot.timestamp;
        event.condition = diag.condition;
        event.evidence = diag.evidence;
        event.severity = Severity::High;

        event.triggered_rules.push_back("Rule: Cross-resource correlation threshold exceeded");
        diagnosis_model_->add_incident(event);

        QString item_text = QString("[%1 s] %2 — %3 evidence signals")
            .arg(snapshot.timestamp.seconds())
            .arg(diag.condition.c_str())
            .arg(diag.evidence.size());

        incident_list_->addItem(item_text);
    }
}

void DiagnosticsPage::on_incident_selected(int row) {
    const auto& incidents = diagnosis_model_->historical_incidents();
    if (row >= 0 && row < static_cast<int>(incidents.size())) {
        const auto& event = incidents[row];
        detail_condition_->setText(QString("Condition: %1").arg(QString::fromStdString(event.condition)));
        detail_severity_->setText("Severity: HIGH");
        detail_duration_->setText("Duration: Instantaneous Alert");

        QString rules = "Triggered Rules:\n";
        for (const auto& rule : event.triggered_rules) {
            rules += QString("- %1\n").arg(QString::fromStdString(rule));
        }
        detail_rules_->setText(rules);

        QString ev_text = "Evidence Summary:\n";
        for (const auto& ev : event.evidence) {
            ev_text += QString("- %1 (Observed: %2, Threshold: %3)\n")
                .arg(QString::fromStdString(ev.explanation))
                .arg(util::format_metric(ev.observed_value).c_str())
                .arg(util::format_metric(ev.threshold).c_str());
        }
        detail_evidence_->setText(ev_text);
    }
}

} // namespace sysscope::ui::qt
