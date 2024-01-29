
#include <stdexcept>
#include <csignal>
#include <atomic>
#include <optional>
#include <thread>
#include <chrono>

#include "NativeMessagingHost.h"
#include "PipeServer.h"
#include "Logger.hpp"
#include "json.hpp"

namespace {
    inline void logError(const std::string& errorMessage) {
        LOG_TAGGED_ERROR(Logger::LogTag::GENERAL, errorMessage);
    }

    inline void logInfo(const std::string& infoMessage) {
        LOG_TAGGED_INFO(Logger::LogTag::GENERAL, infoMessage);
    }
}

class NativeHostServer {
public:
    // Singleton pattern: Get the single instance of the Logger
    static NativeHostServer& getInstance() {
        static NativeHostServer instance;
        return instance;
    }

    void run(std::string serverName) {
        
        logInfo( "NativeHostServer::run START");

        server = std::make_unique<PipeServer>(serverName);
        server->start();
        auto& nativeMessagingHost = NativeMessagingHost::getInstance();
        nativeMessagingHost.start();

        while(!stopRequested) {
            auto jsonResult = server->readRequest();
            if (jsonResult.has_value()) {
                auto obj = jsonResult.value();
                std::string actionName = obj["action"];
                
                // handle only jsonObject with "action" field
                if (!actionName.empty()) {
                    nativeMessagingHost.sendRequest(actionName);
                    auto data = nativeMessagingHost.readResponse();
                    nlohmann::json jsonObject;
                    jsonObject["action"] = actionName;
                    if (data.has_value()) {
                        jsonObject["data"] = data.value();
                    } else {
                        jsonObject["data"] = "";
                    }
                    server->sendResponse(jsonObject);
                } else {

                }

            } else {
                logInfo( "server->readRequest()  timeout");
                // pipe server response timeout
            }
        }
    }

    void stop() {
        stopRequested = true;
        server->stop();
        NativeMessagingHost::getInstance().stop();
    }

private:
    NativeHostServer() {

    }
private:

    std::atomic_bool            stopRequested;
    std::string                 pipeServerName;
    std::unique_ptr<PipeServer> server;
};

void testNativeMessaging() {
        auto& nativeMessagingHost = NativeMessagingHost::getInstance();
        nativeMessagingHost.start();
        while(true) {
            nativeMessagingHost.sendRequest("tabInfo");
            auto response = nativeMessagingHost.readResponse();
            if (response.has_value()) {
                 logInfo("response: " + response.value());
            } else {
                logError("response : {}");
            }
            // Sleep for 3 seconds
            std::this_thread::sleep_for(std::chrono::seconds(3));
        } 
}


// Signal handler to gracefully stop the program on Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received Ctrl+C. Stopping the program..." << std::endl;
        NativeHostServer::getInstance().stop();
        std::exit(0);
    }
}


int main() {
    testNativeMessaging();
    // // Set up signal handler for Ctrl+C
    // std::signal(SIGINT, signalHandler);

    // logInfo("Main Called");

    // NativeHostServer::getInstance().run("com.snapcast.chrome.nativehost.service");

    return 0;
}
