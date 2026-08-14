#include "sysscope/collectors/process.hpp"
#include <sstream>
#include <unordered_set>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#else
#include <unistd.h>
#endif

namespace sysscope::collectors {

ProcessCollector::ProcessCollector(std::shared_ptr<platform::IFileSystemReader> fs_reader)
    : fs_reader_(std::move(fs_reader)) {
#if defined(_WIN32) || defined(_WIN64)
    ticks_per_sec_ = 100;
#else
    long ticks = sysconf(_SC_CLK_TCK);
    ticks_per_sec_ = (ticks > 0) ? ticks : 100;
#endif
}

core::Result<core::ProcessSnapshot> ProcessCollector::collect() {
    auto dir_res = fs_reader_->list_directory_entries("/proc");
    if (dir_res.is_err()) {
        return core::Result<core::ProcessSnapshot>::error(dir_res.error_message());
    }

    // Read total system CPU time from /proc/stat
    uint64_t current_total_system_time = 0;
    auto stat_res = fs_reader_->read_file_lines("/proc/stat");
    if (stat_res.is_ok() && !stat_res.value().empty()) {
        std::istringstream ss(stat_res.value()[0]);
        std::string label;
        uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0;
        if (ss >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal) {
            current_total_system_time = user + nice + system + idle + iowait + irq + softirq + steal;
        }
    }

    core::ProcessSnapshot snapshot{};
    std::unordered_map<int, ProcessStateSnapshot> current_process_states;
    std::unordered_set<int> current_pids;

    for (const auto& entry : dir_res.value()) {
        // Check if entry is a numeric PID directory
        if (entry.empty() || !std::all_of(entry.begin(), entry.end(), ::isdigit)) {
            continue;
        }

        int pid = 0;
        try {
            pid = std::stoi(entry);
        } catch (...) {
            continue;
        }

        std::string pid_stat_path = "/proc/" + entry + "/stat";
        auto pid_stat_res = fs_reader_->read_file_contents(pid_stat_path);
        if (pid_stat_res.is_err()) {
            continue;
        }

        const std::string& stat_content = pid_stat_res.value();
        size_t open_paren = stat_content.find('(');
        size_t close_paren = stat_content.rfind(')');
        if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren) {
            continue;
        }

        core::ProcessInfo proc_info{};
        proc_info.pid = pid;
        proc_info.name = stat_content.substr(open_paren + 1, close_paren - open_paren - 1);

        std::string rest = stat_content.substr(close_paren + 2);
        std::istringstream ss(rest);

        uint64_t cutime = 0, cstime = 0, num_threads = 0, rss_pages = 0;
        int ppid = 0;
        char state = 'R';

        ss >> state >> ppid;
        for (int i = 0; i < 9; ++i) { std::string dummy; ss >> dummy; }
        ss >> proc_info.utime >> proc_info.stime >> cutime >> cstime;
        for (int i = 0; i < 4; ++i) { std::string dummy; ss >> dummy; }
        ss >> num_threads;
        for (int i = 0; i < 2; ++i) { std::string dummy; ss >> dummy; }
        ss >> proc_info.vms_bytes >> rss_pages;

        proc_info.ppid = ppid;
        proc_info.state = state;
        proc_info.threads = static_cast<uint32_t>(num_threads);
        proc_info.rss_bytes = rss_pages * 4096ULL; // Assuming 4KB page size

        // Read cmdline if available
        std::string cmdline_path = "/proc/" + entry + "/cmdline";
        auto cmdline_res = fs_reader_->read_file_contents(cmdline_path);
        if (cmdline_res.is_ok() && !cmdline_res.value().empty()) {
            proc_info.cmdline = cmdline_res.value();
            std::replace(proc_info.cmdline.begin(), proc_info.cmdline.end(), '\0', ' ');
        } else {
            proc_info.cmdline = "[" + proc_info.name + "]";
        }

        uint64_t total_proc_time = proc_info.utime + proc_info.stime;
        current_process_states[pid] = ProcessStateSnapshot{total_proc_time, core::Timestamp::now().nanoseconds()};
        current_pids.insert(pid);

        // Compute process CPU % if we have previous baseline for this PID
        if (has_previous_state_ && previous_total_system_time_ > 0 && current_total_system_time > previous_total_system_time_) {
            auto prev_it = previous_process_states_.find(pid);
            if (prev_it != previous_process_states_.end()) {
                uint64_t proc_delta = total_proc_time - prev_it->second.total_process_cpu_time;
                uint64_t sys_delta = current_total_system_time - previous_total_system_time_;

                if (sys_delta > 0) {
                    proc_info.cpu_percent = std::min(100.0 * static_cast<double>(ticks_per_sec_), 
                                                     100.0 * static_cast<double>(proc_delta) / static_cast<double>(sys_delta));
                }
            }
        }

        if (proc_info.state == 'R') {
            snapshot.running_processes++;
        }
        snapshot.process_list.push_back(proc_info);
    }

    snapshot.total_processes = static_cast<uint32_t>(snapshot.process_list.size());

    // Update state & prune exited PIDs automatically
    previous_process_states_ = std::move(current_process_states);
    previous_total_system_time_ = current_total_system_time;
    has_previous_state_ = true;

    return core::Result<core::ProcessSnapshot>(snapshot);
}

std::unordered_map<int, ProcessNode> ProcessCollector::build_process_tree(const core::ProcessSnapshot& snapshot) const {
    std::unordered_map<int, ProcessNode> tree;
    for (const auto& proc : snapshot.process_list) {
        tree[proc.pid].info = proc;
    }
    for (const auto& proc : snapshot.process_list) {
        if (proc.ppid != 0 && tree.find(proc.ppid) != tree.end()) {
            tree[proc.ppid].children_pids.push_back(proc.pid);
        }
    }
    return tree;
}

} // namespace sysscope::collectors
