#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <stdexcept>
#include <csignal>

#include "NativeMessagingHost.h"
#include "Logger.hpp"

// Signal handler to gracefully stop the program on Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received Ctrl+C. Stopping the program..." << std::endl;
        NativeMessagingHost::getInstance().stop();
        std::exit(0);
    }
}

void native_messaging_test() {
    auto& nativeMessagingHost = NativeMessagingHost::getInstance();
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
}

void logger_test() {
     LOG_TAGGED_ERROR(Logger::LogTag::NETIVE_MESSAGING, "error message test");
     LOG_TAGGED_INFO(Logger::LogTag::NETIVE_MESSAGING, "info message test");
     LOG_TAGGED_WARNING(Logger::LogTag::NETIVE_MESSAGING, "info message test");

     LOG_TAGGED_ERROR(Logger::LogTag::GENERAL, "error message test");
     LOG_TAGGED_INFO(Logger::LogTag::GENERAL, "info message test");
     LOG_TAGGED_WARNING(Logger::LogTag::GENERAL, "info message test");     

}

int main() {
    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

    //native_messaging_test();
    logger_test();

    return 0;
}
