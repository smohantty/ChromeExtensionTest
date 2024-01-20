
#include <stdexcept>
#include <csignal>
#include <atomic>
#include <optional>

#include "NativeMessagingHost.h"
#include "PipeServer.h"
#include "Logger.hpp"
#include "json.hpp"


class NativeHostServer {
public:
    // Singleton pattern: Get the single instance of the Logger
    static NativeHostServer& getInstance() {
        static NativeHostServer instance;
        return instance;
    }

    void run(std::string serverName) {
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


// Signal handler to gracefully stop the program on Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received Ctrl+C. Stopping the program..." << std::endl;
        NativeHostServer::getInstance().stop();
        std::exit(0);
    }
}


int main() {
    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    NativeHostServer::getInstance().run("com.snapcast.chrome.nativehost.service");

    return 0;
}
