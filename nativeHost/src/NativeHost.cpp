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

#define JSON_NO_IO
#include "json.hpp"

using json = nlohmann::json;

class NativeMessagingHost {
public:
    NativeMessagingHost() : stopRequested(false) {
    }

    ~NativeMessagingHost() {
        stop();
    }

    void start() {
        try {
            readThread = std::thread(&NativeMessagingHost::readHandler, this);
            writeThread = std::thread(&NativeMessagingHost::writeHandler, this);
        } catch (const std::exception& ex) {
            logError("Error starting the threads: " + std::string(ex.what()));
        }
    }

    void stop() {
        stopRequested = true;

        requestQueue.notifyAll();
        messageQueue.notifyAll();

        if (readThread.joinable()) {
            readThread.join();
        }

        if (writeThread.joinable()) {
            writeThread.join();
        }

    }

    bool isStopRequested() {
        return stopRequested;
    }

    void sendRequest(const std::string& request) {
        requestQueue.push(request);
    }

    std::optional<std::string> readResponse(std::chrono::milliseconds timeout = READ_RESPONSE_TIMEOUT_MILLISECONDS) {
        return messageQueue.pop(timeout);
    }

private:
    static constexpr std::chrono::milliseconds READ_RESPONSE_TIMEOUT_MILLISECONDS = std::chrono::milliseconds(500);
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

    // Log an error message to the log file
    void logError(const std::string& errorMessage) {
        LOG_TAGGED_ERROR(Logger::LogTag::NETWORK, errorMessage);
    }
};

// Signal handler to gracefully stop the program on Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received Ctrl+C. Stopping the program..." << std::endl;
        std::exit(0);
    }
}

int main() {
    // Set stdin and stdout to binary mode
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    std::cin >> std::noskipws;

    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    NativeMessagingHost nativeMessagingHost;
    nativeMessagingHost.start();

    while (!nativeMessagingHost.isStopRequested()) {
        nativeMessagingHost.sendRequest("urlInfo");

        auto result = nativeMessagingHost.readResponse();

        if (result.has_value()) {
            // response = result.value()
        } else {
            // response timeout
        }
    }

    return 0;
}
