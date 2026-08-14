#pragma once

#include "sysscope/ipc/ipc.hpp"
#include <atomic>
#include <thread>
#include <mutex>

namespace sysscope::ipc {

class RealIpcServer : public IIpcServer {
public:
    RealIpcServer() = default;
    ~RealIpcServer() override;

    bool start(const std::string& socket_path) override;
    void stop() override;
    bool is_running() const override;

private:
    std::string socket_path_;
    std::atomic<bool> running_{false};
    int server_fd_{-1};
    std::thread server_thread_;
};

class RealIpcClient : public IIpcClient {
public:
    RealIpcClient() = default;
    ~RealIpcClient() override;

    bool connect(const std::string& socket_path) override;
    void disconnect() override;
    core::Result<std::string> send_request(IpcCommandType cmd, const std::string& payload = "") override;

private:
    std::string socket_path_;
    int client_fd_{-1};
    bool connected_{false};
};

} // namespace sysscope::ipc
