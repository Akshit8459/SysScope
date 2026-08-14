#pragma once

#include "sysscope/core/result.hpp"
#include "sysscope/core/snapshot.hpp"
#include <string>
#include <vector>
#include <memory>

namespace sysscope::ipc {

enum class IpcCommandType {
    Ping = 1,
    GetSnapshot = 2,
    GetDiagnostics = 3
};

struct IpcMessage {
    IpcCommandType command{IpcCommandType::Ping};
    std::string payload;
};

class IIpcServer {
public:
    virtual ~IIpcServer() = default;

    virtual bool start(const std::string& socket_path) = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
};

class IIpcClient {
public:
    virtual ~IIpcClient() = default;

    virtual bool connect(const std::string& socket_path) = 0;
    virtual void disconnect() = 0;
    virtual core::Result<std::string> send_request(IpcCommandType cmd, const std::string& payload = "") = 0;
};

class MockIpcServer : public IIpcServer {
public:
    MockIpcServer() = default;
    ~MockIpcServer() override = default;

    bool start(const std::string& socket_path) override {
        socket_path_ = socket_path;
        running_ = true;
        return true;
    }

    void stop() override {
        running_ = false;
    }

    bool is_running() const override {
        return running_;
    }

private:
    std::string socket_path_;
    bool running_{false};
};

class MockIpcClient : public IIpcClient {
public:
    MockIpcClient() = default;
    ~MockIpcClient() override = default;

    bool connect(const std::string& socket_path) override {
        socket_path_ = socket_path;
        connected_ = true;
        return true;
    }

    void disconnect() override {
        connected_ = false;
    }

    core::Result<std::string> send_request(IpcCommandType cmd, const std::string& payload = "") override {
        (void)payload;
        if (!connected_) return core::Result<std::string>::error("IPC Client disconnected.");
        if (cmd == IpcCommandType::Ping) {
            return core::Result<std::string>("PONG");
        }
        return core::Result<std::string>("OK");
    }

private:
    std::string socket_path_;
    bool connected_{false};
};

} // namespace sysscope::ipc
