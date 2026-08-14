#pragma once

#include "sysscope/platform/netlink_reader.hpp"

namespace sysscope::platform {

class RealNetlinkReader : public INetlinkReader {
public:
    RealNetlinkReader() = default;
    ~RealNetlinkReader() override = default;

    [[nodiscard]] bool is_available() const override;
    [[nodiscard]] core::Result<std::vector<NetlinkLinkStats>> query_link_stats() override;
};

} // namespace sysscope::platform
