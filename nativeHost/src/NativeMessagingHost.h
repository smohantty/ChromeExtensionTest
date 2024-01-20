#ifndef NATIVE_MESSAGING_HOST_H
#define NATIVE_MESSAGING_HOST_H

#include <string>
#include <optional>
#include <chrono>
#include <memory>

class NativeMessagingHost {
public:
    // Singleton pattern: Get the single instance of the NativeMessagingHost
    static NativeMessagingHost& getInstance();

    void start();

    void stop();

    bool isStopRequested();

    void sendRequest(const std::string& request);

    std::optional<std::string> readResponse(std::chrono::milliseconds timeout = READ_RESPONSE_TIMEOUT_MILLISECONDS);
    
private:
    static constexpr std::chrono::milliseconds READ_RESPONSE_TIMEOUT_MILLISECONDS = std::chrono::milliseconds(500);
    
    // Private constructor to enforce singleton pattern
    NativeMessagingHost();

    class NativeMessagingHostImpl;
    std::unique_ptr<NativeMessagingHostImpl> mImpl;
};

#endif  // NATIVE_MESSAGING_HOST_H