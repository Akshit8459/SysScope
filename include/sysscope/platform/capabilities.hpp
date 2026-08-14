#pragma once

#include <string>

namespace sysscope::platform {

struct PlatformCapabilities {
    bool cpu{false};
    bool memory{false};
    bool process{false};
    bool network{false};
    bool disk{false};
    bool thermal{false};
    bool psi{false};
    bool nvidia_gpu{false};
    bool qualcomm_gpu{false};
    bool npu{false};
    bool dsp{false};
    bool power{false};

    [[nodiscard]] std::string summarize() const;
};

} // namespace sysscope::platform
