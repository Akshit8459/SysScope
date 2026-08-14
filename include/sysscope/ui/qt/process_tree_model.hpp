#pragma once

#include "sysscope/core/snapshot.hpp"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QAbstractItemModel>)
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#endif

namespace sysscope::ui::qt {

struct ProcessNode {
    int pid{0};
    int ppid{0};
    std::string name;
    double cpu_percent{0.0};
    uint64_t rss_bytes{0};
    char state{'R'};
    int num_threads{1};

    ProcessNode* parent{nullptr};
    std::vector<std::unique_ptr<ProcessNode>> children;
};

class ProcessTreeModel
#if defined(SYSSCOPE_HAS_QT) || __has_include(<QAbstractItemModel>)
    : public QAbstractItemModel
#endif
{
#if defined(SYSSCOPE_HAS_QT) || __has_include(<QAbstractItemModel>)
    Q_OBJECT
#endif

public:
#if defined(SYSSCOPE_HAS_QT) || __has_include(<QAbstractItemModel>)
    explicit ProcessTreeModel(QObject* parent = nullptr);
    ~ProcessTreeModel() override = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
#else
    ProcessTreeModel() = default;
#endif

    void update_processes(const std::vector<sysscope::core::ProcessInfo>& processes);

private:
    std::unique_ptr<ProcessNode> root_;
    std::unordered_map<int, ProcessNode*> node_map_;
};

} // namespace sysscope::ui::qt
