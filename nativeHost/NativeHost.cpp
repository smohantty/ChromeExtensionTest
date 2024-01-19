#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <stdexcept>
#include <csignal>

#define JSON_NO_IO
#include "json.hpp"

using json = nlohmann::json;

class NativeMessagingHost {
public:
    NativeMessagingHost() : stopRequested(false) {
        logFileName = "/tmp/native_messaging_log.txt"; // Specify your log file name here
        logFile.open(logFileName, std::ios::app); // Open log file in append mode
        if (!logFile.is_open()) {
            std::cerr << "Error opening log file: " << logFileName << std::endl;
        } else {
            logError("Log file open sucess : ");
        }
    }

    ~NativeMessagingHost() {
        stop();
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    // Start the message handling thread
    void start() {
        try {
            messageThread = std::thread(&NativeMessagingHost::messageHandler, this);
        } catch (const std::exception& ex) {
            logError("Error starting the message handling thread: " + std::string(ex.what()));
        }
    }

    // Stop the message handling thread
    void stop() {
        stopRequested = true;

        // If the message thread is joinable, wait for it to finish
        if (messageThread.joinable()) {
            messageThread.join();
        }
    }

    // Send a request to the Chrome extension
    void sendRequest(const std::string& request) {
        try {
            std::scoped_lock lock(inputMutex);
            sendRequestInternal(request);
        } catch (const std::exception& ex) {
            logError("Error sending request: " + std::string(ex.what()));
        }
    }

    // Asynchronously process a message
    std::future<std::optional<std::string>> processMessageAsync(const std::string& message) {
        return std::async(std::launch::async, [this, message] {
            try {
                return processMessage(message);
            } catch (const std::exception& ex) {
                logError("Error processing message: " + std::string(ex.what()));
                return std::optional<std::string>();
            }
        });
    }

private:
    std::thread messageThread;
    std::mutex inputMutex;
    std::mutex outputMutex;
    bool stopRequested;
    std::ofstream logFile;
    std::string logFileName;

    std::optional<std::string> processMessage(const std::string& input) {
        logError("Received message from Extension : " + std::string(input));
        // Your message processing logic goes here
        return "Hello from Native Messaging host: " + input;
    }

    void sendRequestInternal(const std::string& request) {
        // Serialize the request as JSON
        json requestJson;
        requestJson["action"] = request;
        std::string serializedRequest = requestJson.dump();

        // Send the request length
        int request_length = serializedRequest.size();
        std::cout.write(reinterpret_cast<const char*>(&request_length), sizeof(request_length));

        // Send the serialized request
        std::cout.write(serializedRequest.c_str(), request_length);
        std::cout.flush();

        //logError("have sent a message to chrome extension " + std::string(serializedRequest));
    }

    void messageHandler() {
        while (!stopRequested) {
            // Read the message length
            int message_length;
            std::cin.read(reinterpret_cast<char*>(&message_length), sizeof(message_length));

            if (std::cin.eof() || std::cin.fail() || message_length <= 0) {
                logError("failed to read message length");
                // Handle error reading message length or reaching the end
                break;
            }

            // Read the actual message
            std::string message;
            message.resize(message_length);
            std::cin.read(&message[0], message_length);

            if (std::cin.eof() || std::cin.fail()) {
                logError("failed to read message");
                // Handle error reading message or reaching the end
                break;
            }

            // Asynchronously process the message and send the response
            std::future<std::optional<std::string>> responseFuture = processMessageAsync(message);
            std::optional<std::string> response = responseFuture.get();

            if (!response) {
                // Handle error processing the message
                break;
            }

            // Send the response length
            int response_length = response.value().size();
            {
                std::scoped_lock lock(outputMutex);
                // Check if writing to standard output fails
                if (!std::cout.write(reinterpret_cast<const char*>(&response_length), sizeof(response_length))) {
                    // Handle error writing response length
                    logError("Error writing response length");
                    break;
                }
            }

            // Send the response
            {
                std::scoped_lock lock(outputMutex);
                // Check if writing to standard output fails
                if (!std::cout.write(response->c_str(), response_length)) {
                    // Handle error writing response
                    logError("Error writing response");
                    break;
                }
                std::cout.flush();
            }
        }
    }

    // Log an error message to the log file
    void logError(const std::string& errorMessage) {
        std::scoped_lock lock(outputMutex);
        if (logFile.is_open()) {
            logFile << "Error: " << errorMessage << std::endl;
            logFile.flush();
        } else {
            std::cerr << "Error: " << errorMessage << std::endl;
        }
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

    // Run continuously until interrupted
    while (true) {
        // Your main logic goes here
        // ...

        // Example: Sending a request to the extension
        nativeMessagingHost.sendRequest("urlInfo");

        // Wait for a while (for demonstration purposes)
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
