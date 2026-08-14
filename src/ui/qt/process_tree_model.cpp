#include "sysscope/ui/qt/process_tree_model.hpp"
#include "sysscope/util/format.hpp"

namespace sysscope::ui::qt {

#if defined(SYSSCOPE_HAS_QT) || __has_include(<QAbstractItemModel>)

ProcessTreeModel::ProcessTreeModel(QObject* parent)
    : QAbstractItemModel(parent) {
    root_ = std::make_unique<ProcessNode>();
    root_->pid = 0;
    root_->name = "system_root";
}

QModelIndex ProcessTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) return {};

    ProcessNode* parent_node = nullptr;
    if (!parent.isValid()) {
        parent_node = root_.get();
    } else {
        parent_node = static_cast<ProcessNode*>(parent.internalPointer());
    }

    if (row >= 0 && row < static_cast<int>(parent_node->children.size())) {
        return createIndex(row, column, parent_node->children[row].get());
    }
    return {};
}

QModelIndex ProcessTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) return {};

    auto* child_node = static_cast<ProcessNode*>(child.internalPointer());
    ProcessNode* parent_node = child_node->parent;

    if (parent_node == root_.get() || !parent_node) return {};

    ProcessNode* grand_parent = parent_node->parent;
    if (!grand_parent) return {};

    int row = 0;
    for (size_t i = 0; i < grand_parent->children.size(); ++i) {
        if (grand_parent->children[i].get() == parent_node) {
            row = static_cast<int>(i);
            break;
        }
    }
    return createIndex(row, 0, parent_node);
}

int ProcessTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) return 0;

    ProcessNode* parent_node = nullptr;
    if (!parent.isValid()) {
        parent_node = root_.get();
    } else {
        parent_node = static_cast<ProcessNode*>(parent.internalPointer());
    }

    return static_cast<int>(parent_node->children.size());
}

int ProcessTreeModel::columnCount(const QModelIndex& /*parent*/) const {
    return 6; // PID, PPID, Name, CPU %, RSS, State
}

QVariant ProcessTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return {};

    auto* node = static_cast<ProcessNode*>(index.internalPointer());
    switch (index.column()) {
        case 0: return node->pid;
        case 1: return node->ppid;
        case 2: return QString::fromStdString(node->name);
        case 3: return QString("%1%").arg(util::format_metric(node->cpu_percent).c_str());
        case 4: return QString::fromStdString(util::format_bytes(node->rss_bytes));
        case 5: return QString(QChar(node->state));
        default: return {};
    }
}

QVariant ProcessTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return "PID";
            case 1: return "PPID";
            case 2: return "Process Name";
            case 3: return "CPU %";
            case 4: return "RSS Memory";
            case 5: return "State";
            default: return {};
        }
    }
    return {};
}

void ProcessTreeModel::update_processes(const std::vector<sysscope::core::ProcessInfo>& processes) {
    beginResetModel();
    root_->children.clear();
    node_map_.clear();

    for (const auto& proc : processes) {
        auto node = std::make_unique<ProcessNode>();
        node->pid = proc.pid;
        node->ppid = proc.ppid;
        node->name = proc.name;
        node->cpu_percent = proc.cpu_percent;
        node->rss_bytes = proc.rss_bytes;
        node->state = proc.state;
        node->num_threads = proc.threads;
        node->parent = root_.get();

        node_map_[proc.pid] = node.get();
        root_->children.push_back(std::move(node));
    }
    endResetModel();
}

#else

void ProcessTreeModel::update_processes(const std::vector<sysscope::core::ProcessInfo>& processes) {
    root_ = std::make_unique<ProcessNode>();
    node_map_.clear();
    for (const auto& proc : processes) {
        auto node = std::make_unique<ProcessNode>();
        node->pid = proc.pid;
        node->ppid = proc.ppid;
        node->name = proc.name;
        node->cpu_percent = proc.cpu_percent;
        node->rss_bytes = proc.rss_bytes;
        node->state = proc.state;
        node->num_threads = proc.threads;
        node->parent = root_.get();

        node_map_[proc.pid] = node.get();
        root_->children.push_back(std::move(node));
    }
}

#endif

} // namespace sysscope::ui::qt
