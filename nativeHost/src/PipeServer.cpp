#include "PipeServer.h"
#include "Logger.hpp"

#include <thread>
#include <atomic>
#include "ConcurrentQueue.hpp"

using json = nlohmann::json; 



namespace {
    inline void logError(const std::string& errorMessage) {
        LOG_TAGGED_ERROR(Logger::LogTag::PIPE_SERVER, errorMessage);
    }

    inline void logInfo(const std::string& infoMessage) {
        LOG_TAGGED_INFO(Logger::LogTag::PIPE_SERVER, infoMessage);
    }    
}

class PipeServerInterface {
public:
    virtual ~PipeServerInterface() {};

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::string readData() const = 0;
    virtual void writeData(const std::string& data) const = 0;
};


#ifdef _WIN32
class WindowsPipeServer : public PipeServerInterface {
public:

};
#else

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring> // for strerror

class LinuxPipeServer : public PipeServerInterface {
public:
    ~LinuxPipeServer() override {
        // No explicit logic in the destructor
    }

    LinuxPipeServer(const std::string& pipeName) : PipeServerInterface(), pipeName(pipeName), fd(0) {}

    void start() override {
        if (mkfifo(pipeName.c_str(), 0666) == -1) {
            throw std::runtime_error("Error creating named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }

        fd = open(pipeName.c_str(), O_RDWR);
        if (fd == -1) {
            throw std::runtime_error("Error opening named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }
    }

    void stop() override {
        if (close(fd) == -1) {
            throw std::runtime_error("Error closing named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }

        if (unlink(pipeName.c_str()) == -1) {
            throw std::runtime_error("Error unlinking named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }
    }

    std::string readData() const override {
        char buffer[1025]; // Extra space for null terminator
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);

        if (bytesRead == -1) {
            throw std::runtime_error("Error reading from named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }

        buffer[bytesRead] = '\0'; // Null-terminate the string
        return std::string(buffer);
    }

    void writeData(const std::string& data) const override {
        ssize_t bytesWritten = write(fd, data.c_str(), data.size());

        if (bytesWritten == -1) {
            throw std::runtime_error("Error writing to named pipe '" + pipeName + "': " + std::string(strerror(errno)));
        }
    }

private:
    std::string pipeName;
    int fd;
};


#endif

class PipeServerImpl {
public:
    PipeServerImpl(const std::string& pipeName):stopRequested(false) {
        #ifdef _WIN32
            mInterface = std::make_unique<WindowsPipeServer>(pipeName);
        #else
            mInterface = std::make_unique<LinuxPipeServer>(pipeName);
        #endif
    }

    void start() {

        try {
            mInterface->start();
        } catch (const std::exception& ex) {
            logError("Exception: " + std::string(ex.what()));
        }

        // Start threads for sending and receiving
        sendThread = std::thread(&PipeServerImpl::sendThreadFunction, this);
        receiveThread = std::thread(&PipeServerImpl::receiveThreadFunction, this);
    }

    void stop() {
        
        stopRequested = true;

        sendQueue.notifyAll();
        receiveQueue.notifyAll();

        if (sendThread.joinable()) {
            sendThread.join();
        }

        if (receiveThread.joinable()) {
            receiveThread.join();
        }        

        try {
            mInterface->stop();
        } catch (const std::exception& ex) {
            logError("Exception: " + std::string(ex.what()));
        }
    }

    void sendResponse(const nlohmann::json& response) {
        sendQueue.push(response);
    }
        
    std::optional<nlohmann::json> readRequest(std::chrono::milliseconds timeout) {
        return receiveQueue.pop(timeout);
    }    


    ~PipeServerImpl() {

    }
private:
    void sendThreadFunction() {
        while (!stopRequested) {
            try {
                auto result = sendQueue.pop(REQUEST_QUEUE_READ_TIMEOUT_MILLISECONDS);
                if (result.has_value()) {
                    std::string serializedData = result.value().dump();
                    mInterface->writeData(serializedData);
                }
            } catch (const std::exception& ex) {
                logError("Exception: " + std::string(ex.what()));
            }
        }
    }

    void receiveThreadFunction() {
        while (!stopRequested) {
            try {
                auto buffer = mInterface->readData();
                json receivedData = json::parse(buffer);

                receiveQueue.push(std::move(receivedData));

            } catch (const std::exception& ex) {
                logError("Exception: " + std::string(ex.what()));
            }
        }
    }

private:
    static constexpr std::chrono::milliseconds REQUEST_QUEUE_READ_TIMEOUT_MILLISECONDS = std::chrono::milliseconds(1000);
    std::atomic_bool stopRequested;
    std::unique_ptr<PipeServerInterface> mInterface;
    std::thread sendThread;
    std::thread receiveThread;
    ConcurrentQueue<json> sendQueue;
    ConcurrentQueue<json> receiveQueue;    
};

PipeServer::PipeServer(const std::string& pipeName): mImpl(std::make_unique<PipeServerImpl>(pipeName)) {

}

PipeServer::~PipeServer() = default;
    
void PipeServer::start() {
    mImpl->start();
}


void PipeServer::stop() {
    mImpl->stop();
}

void PipeServer::sendResponse(const nlohmann::json& request) {
    mImpl->sendResponse(request);
}
    
std::optional<nlohmann::json> PipeServer::readRequest(std::chrono::milliseconds timeout) {
    return mImpl->readRequest(timeout);
}