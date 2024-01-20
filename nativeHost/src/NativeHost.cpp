#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <stdexcept>
#include <csignal>

#include "NativeMessagingHost.h"

// Signal handler to gracefully stop the program on Ctrl+C
void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received Ctrl+C. Stopping the program..." << std::endl;
        NativeMessagingHost::getInstance().stop();
        std::exit(0);
    }
}

int main() {
    // Set up signal handler for Ctrl+C
    std::signal(SIGINT, signalHandler);

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

    return 0;
}
