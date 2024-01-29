#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <stdexcept>
#include <csignal>
#include <queue>

#include "Logger.hpp"
#include "ConcurrentQueue.hpp"
#include "NativeMessagingHost.h"

#define JSON_NO_IO
#include "json.hpp"

using json = nlohmann::json;

namespace {
    inline void logError(const std::string& errorMessage) {
        LOG_TAGGED_ERROR(Logger::LogTag::NETIVE_MESSAGING, errorMessage);
    }

    inline void logInfo(const std::string& infoMessage) {
        LOG_TAGGED_INFO(Logger::LogTag::NETIVE_MESSAGING, infoMessage);
    }
}

class NativeMessagingHost::NativeMessagingHostImpl {
public:
    NativeMessagingHostImpl() : stopRequested(false) {
        setIOStreamsToBinary();
    }

    ~NativeMessagingHostImpl() {
        stop();
    }

    void start() {
        try {
            readThread = std::thread(&NativeMessagingHostImpl::readHandler, this);
            writeThread = std::thread(&NativeMessagingHostImpl::writeHandler, this);
        } catch (const std::exception& ex) {
            logError("Error starting the threads: " + std::string(ex.what()));
        }
    }

    void stop() {
        stopRequested = true;
        logInfo("STOP start");
        requestQueue.notifyAll();
        messageQueue.notifyAll();

        if (readThread.joinable()) {
            readThread.join();
        }

        if (writeThread.joinable()) {
            writeThread.join();
        }
        
        logInfo("STOP end");

    }

    bool isStopRequested() {
        return stopRequested;
    }

    void sendRequest(const std::string& request) {
        requestQueue.push(request);
    }

    std::optional<std::string> readResponse(std::chrono::milliseconds timeout) {
        return messageQueue.pop(timeout);
    }

private:
    static constexpr std::chrono::milliseconds REQUEST_QUEUE_READ_TIMEOUT_MILLISECONDS = std::chrono::milliseconds(1000);
    std::thread readThread;
    std::thread writeThread;
    std::atomic_bool stopRequested;
    std::ofstream logFile;
    std::string logFileName;
    std::mutex logFileMutex;
    ConcurrentQueue<std::string> requestQueue;
    ConcurrentQueue<std::string> messageQueue;

    void readHandler() {
        while (!stopRequested) {
            int message_length;
            std::cin.read(reinterpret_cast<char*>(&message_length), sizeof(message_length));

            if (std::cin.eof() || std::cin.fail() || message_length <= 0) {
                logError("failed to read message length");
                break;
            }

            std::string message;
            message.resize(message_length);
            std::cin.read(&message[0], message_length);

            if (std::cin.eof() || std::cin.fail()) {
                logError("failed to read message");
                break;
            }

            messageQueue.push(message);
        }
    }

    void writeHandler() {
        while (!stopRequested) {
            auto result = requestQueue.pop(REQUEST_QUEUE_READ_TIMEOUT_MILLISECONDS);

            if (result.has_value()) {
                std::string request = result.value();

                json requestJson;
                requestJson["action"] = request;
                std::string serializedRequest = requestJson.dump();

                int request_length = serializedRequest.size();
                std::cout.write(reinterpret_cast<const char*>(&request_length), sizeof(request_length));
                std::cout.write(serializedRequest.c_str(), request_length);
                std::cout.flush();
            }
        }
    }

    void setIOStreamsToBinary() {
        // Set stdin and stdout to binary mode
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(nullptr);
        std::cout.tie(nullptr);
        std::cin >> std::noskipws;
    }
};

NativeMessagingHost& NativeMessagingHost::getInstance() {
    static NativeMessagingHost instance;
    return instance;
}

NativeMessagingHost::NativeMessagingHost() : mImpl(std::make_unique<NativeMessagingHost::NativeMessagingHostImpl>()) { }

void NativeMessagingHost::start() {
    mImpl->start();
}

void NativeMessagingHost::stop() {
    mImpl->stop();
}

bool NativeMessagingHost::isStopRequested() {
    return mImpl->isStopRequested();
}

void NativeMessagingHost::sendRequest(const std::string& request) {
    mImpl->sendRequest(request);
}

std::optional<std::string> NativeMessagingHost::readResponse(std::chrono::milliseconds timeout) {
    return mImpl->readResponse(timeout);
}
