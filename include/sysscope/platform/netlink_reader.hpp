#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/collectors/network.hpp"
#include <string>
#include <vector>

namespace sysscope::platform {

struct NetlinkLinkStats {
    std::string interface_name;
    uint64_t rx_bytes{0};
    uint64_t tx_bytes{0};
    uint64_t rx_packets{0};
    uint64_t tx_packets{0};
    uint64_t rx_errors{0};
    uint64_t tx_errors{0};
    uint64_t rx_drops{0};
    uint64_t tx_drops{0};
};

class INetlinkReader {
public:
    virtual ~INetlinkReader() = default;

    [[nodiscard]] virtual bool is_available() const = 0;
    [[nodiscard]] virtual core::Result<std::vector<NetlinkLinkStats>> query_link_stats() = 0;
};

class MockNetlinkReader : public INetlinkReader {
public:
    MockNetlinkReader(bool available = true) : available_(available) {}
    ~MockNetlinkReader() override = default;

    [[nodiscard]] bool is_available() const override { return available_; }
    void add_mock_link(NetlinkLinkStats stats) { mock_links_.push_back(std::move(stats)); }

    [[nodiscard]] core::Result<std::vector<NetlinkLinkStats>> query_link_stats() override {
        if (!available_) {
            return core::Result<std::vector<NetlinkLinkStats>>::error("Netlink Socket Unavailable");
        }
        return core::Result<std::vector<NetlinkLinkStats>>(mock_links_);
    }

private:
    bool available_{true};
    std::vector<NetlinkLinkStats> mock_links_;
};

} // namespace sysscope::platform
