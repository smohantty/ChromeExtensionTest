#ifndef PIPE_SERVER_H
#define PIPE_SERVER_H

#include <memory>
#include <functional>

#include "json.hpp"

class PipeServerImpl;

class PipeServer {
public:
    PipeServer(const std::string& pipeName);
    
    void onReceiveData(std::function<void(const nlohmann::json&)> block);

    void start();
    void stop();

private:
    // Delete copy and move operations
    PipeServer(const PipeServer&) = delete;
    PipeServer& operator=(const PipeServer&) = delete;
    PipeServer(PipeServer&&) = delete;
    PipeServer& operator=(PipeServer&&) = delete;

    std::unique_ptr<PipeServerImpl> mImpl;
};

#endif  // PIPE_SERVER_H