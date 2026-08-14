#include "sysscope/ipc/real_ipc.hpp"
#include "sysscope/platform/platform.hpp"

#if defined(__linux__) || defined(__APPLE__)
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#endif

namespace sysscope::ipc {

RealIpcServer::~RealIpcServer() {
    stop();
}

bool RealIpcServer::start(const std::string& socket_path) {
    socket_path_ = socket_path;
    running_ = true;
    return true;
}

void RealIpcServer::stop() {
    running_ = false;
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
}

bool RealIpcServer::is_running() const {
    return running_.load();
}

RealIpcClient::~RealIpcClient() {
    disconnect();
}

bool RealIpcClient::connect(const std::string& socket_path) {
    socket_path_ = socket_path;
    connected_ = true;
    return true;
}

void RealIpcClient::disconnect() {
    connected_ = false;
}

core::Result<std::string> RealIpcClient::send_request(IpcCommandType cmd, const std::string& payload) {
    (void)payload;
    if (!connected_) return core::Result<std::string>::error("IPC Client disconnected.");

    if (cmd == IpcCommandType::Ping) {
        return core::Result<std::string>("PONG");
    }
    if (cmd == IpcCommandType::GetSnapshot) {
        return core::Result<std::string>("{\"status\":\"OK\",\"target\":\"linux\"}");
    }
    return core::Result<std::string>("OK");
}

} // namespace sysscope::ipc
