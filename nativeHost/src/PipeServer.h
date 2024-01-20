#ifndef PIPE_SERVER_H
#define PIPE_SERVER_H

#include <memory>
#include <optional>

#include "json.hpp"

class PipeServerImpl;

class PipeServer {
public:
    PipeServer(const std::string& pipeName);
    ~PipeServer();
    
    void sendResponse(const nlohmann::json& request);

    std::optional<nlohmann::json> readRequest(std::chrono::milliseconds timeout = READ_REQUEST_TIMEOUT_MILLISECONDS);

    void start();
    void stop();

private:
    static constexpr std::chrono::milliseconds READ_REQUEST_TIMEOUT_MILLISECONDS = std::chrono::milliseconds(2000);
    // Delete copy and move operations
    PipeServer(const PipeServer&) = delete;
    PipeServer& operator=(const PipeServer&) = delete;
    PipeServer(PipeServer&&) = delete;
    PipeServer& operator=(PipeServer&&) = delete;

    std::unique_ptr<PipeServerImpl> mImpl;
};

#endif  // PIPE_SERVER_H