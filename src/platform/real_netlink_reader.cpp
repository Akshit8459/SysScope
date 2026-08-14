#include "sysscope/platform/real_netlink_reader.hpp"
#include "sysscope/platform/platform.hpp"

#if defined(__linux__)
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#endif

namespace sysscope::platform {

bool RealNetlinkReader::is_available() const {
    return is_linux_platform();
}

core::Result<std::vector<NetlinkLinkStats>> RealNetlinkReader::query_link_stats() {
    if (!is_linux_platform()) {
        return core::Result<std::vector<NetlinkLinkStats>>::error("rtnetlink sockets are only supported on Linux platform targets.");
    }

#if defined(__linux__)
    int fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (fd < 0) {
        return core::Result<std::vector<NetlinkLinkStats>>::error("Failed to open AF_NETLINK route socket.");
    }

    struct {
        struct nlmsghdr nlh;
        struct ifinfomsg ifm;
    } req{};

    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nlh.nlmsg_type = RTM_GETLINK;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.ifm.ifi_family = AF_UNSPEC;

    if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0) {
        close(fd);
        return core::Result<std::vector<NetlinkLinkStats>>::error("Failed to send RTM_GETLINK netlink request.");
    }

    std::vector<NetlinkLinkStats> stats_list;
    char buffer[8192];
    bool done = false;

    while (!done) {
        ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
        if (len <= 0) break;

        for (struct nlmsghdr* nlh = reinterpret_cast<struct nlmsghdr*>(buffer);
             NLMSG_OK(nlh, static_cast<size_t>(len));
             nlh = NLMSG_NEXT(nlh, len)) {

            if (nlh->nlmsg_type == NLMSG_DONE) {
                done = true;
                break;
            }
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                done = true;
                break;
            }
            if (nlh->nlmsg_type != RTM_NEWLINK) continue;

            struct ifinfomsg* ifm = reinterpret_cast<struct ifinfomsg*>(NLMSG_DATA(nlh));
            struct rtattr* rta = IFLA_RTA(ifm);
            int rta_len = IFLA_PAYLOAD(nlh);

            NetlinkLinkStats link_stat{};
            bool has_stats = false;

            for (; RTA_OK(rta, rta_len); rta = RTA_NEXT(rta, rta_len)) {
                if (rta->rta_type == IFLA_IFNAME) {
                    link_stat.interface_name = reinterpret_cast<char*>(RTA_DATA(rta));
                } else if (rta->rta_type == IFLA_STATS64) {
                    auto* s64 = reinterpret_cast<struct rtnl_link_stats64*>(RTA_DATA(rta));
                    link_stat.rx_bytes = s64->rx_bytes;
                    link_stat.tx_bytes = s64->tx_bytes;
                    link_stat.rx_packets = s64->rx_packets;
                    link_stat.tx_packets = s64->tx_packets;
                    link_stat.rx_errors = s64->rx_errors;
                    link_stat.tx_errors = s64->tx_errors;
                    has_stats = true;
                } else if (rta->rta_type == IFLA_STATS && !has_stats) {
                    auto* s32 = reinterpret_cast<struct rtnl_link_stats*>(RTA_DATA(rta));
                    link_stat.rx_bytes = s32->rx_bytes;
                    link_stat.tx_bytes = s32->tx_bytes;
                    link_stat.rx_packets = s32->rx_packets;
                    link_stat.tx_packets = s32->tx_packets;
                    link_stat.rx_errors = s32->rx_errors;
                    link_stat.tx_errors = s32->tx_errors;
                    has_stats = true;
                }
            }

            if (!link_stat.interface_name.empty() && has_stats) {
                stats_list.push_back(link_stat);
            }
        }
    }

    close(fd);
    return core::Result<std::vector<NetlinkLinkStats>>(stats_list);
#else
    return core::Result<std::vector<NetlinkLinkStats>>::error("rtnetlink unavailable.");
#endif
}

} // namespace sysscope::platform
