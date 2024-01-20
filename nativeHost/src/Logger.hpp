#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

class Logger {
public:
    // Enumeration for log levels (INFO, WARNING, ERROR)
    enum class LogLevel { INFO, WARNING, ERROR };
    // Enumeration for log tags (GENERAL, NETIVE_MESSAGING, FILE_IO)
    enum class LogTag { GENERAL, NETIVE_MESSAGING, FILE_IO };

    // Convenient macros for logging with tags
#define LOG_TAGGED_INFO(tag, message) Logger::getInstance().log(Logger::LogLevel::INFO, tag, message, __FILE__, __LINE__)
#define LOG_TAGGED_WARNING(tag, message) Logger::getInstance().log(Logger::LogLevel::WARNING, tag, message, __FILE__, __LINE__)
#define LOG_TAGGED_ERROR(tag, message) Logger::getInstance().log(Logger::LogLevel::ERROR, tag, message, __FILE__, __LINE__)

    // Singleton pattern: Get the single instance of the Logger
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Log a message with a specific level, tag, and source file information
    void log(LogLevel level, LogTag tag, const std::string& message, const char* file, int line) {
        // Lock the log file to ensure thread safety
        std::scoped_lock lock(logFileMutex_);
        // Check if the log file is open
        if (logFile_.is_open()) {
            // Write the formatted log message to the log file
            logFile_ << coloredLogString(level) << coloredLogTagString(tag) << " "
                     << "[" << file << ":" << line << "] " << message << std::endl;
            logFile_.flush();  // Flush the log file to ensure immediate write
        } else {
            // Log to standard error if the log file is not open
            std::cerr << coloredLogString(level) << coloredLogTagString(tag) << " "
                      << "[" << file << ":" << line << "] " << message << std::endl;
        }
    }

private:
    // Private constructor to enforce singleton pattern
    Logger() {
        // Initialize log file name
        logFileName_ = "/tmp/native_messaging_log.txt";
        // Open log file for append mode
        logFile_.open(logFileName_, std::ios::app);
        // Check if the log file failed to open
        if (!logFile_.is_open()) {
            std::cerr << "Error opening log file: " << logFileName_ << std::endl;
        }
    }

    // Private destructor to clean up resources
    ~Logger() {
        // Close the log file if it is open
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }

    std::string logFileName_;       // Log file name
    std::ofstream logFile_;         // Log file stream
    std::mutex logFileMutex_;       // Mutex to ensure thread safety for log file access

    // Function to generate colored log level string based on the log level
    std::string coloredLogString(LogLevel level) const {
        switch (level) {
            case LogLevel::INFO:
                return "\x1B[32m[INFO]\x1B[0m";      // Green color for INFO
            case LogLevel::WARNING:
                return "\x1B[33m[WARNING]\x1B[0m";   // Yellow color for WARNING
            case LogLevel::ERROR:
                return "\x1B[31m[ERROR]\x1B[0m";     // Red color for ERROR
            default:
                return "[UNKNOWN]";
        }
    }

    // Function to generate colored log tag string based on the log tag
    std::string coloredLogTagString(LogTag tag) const {
        switch (tag) {
            case LogTag::GENERAL:
                return "\x1B[36m[GENERAL]\x1B[0m";   // Cyan color for GENERAL
            case LogTag::NETIVE_MESSAGING:
                return "\x1B[35m[NETIVE_MESSAGING]\x1B[0m";   // Magenta color for NETIVE_MESSAGING
            case LogTag::FILE_IO:
                return "\x1B[34m[FILE_IO]\x1B[0m";   // Blue color for FILE_IO
            default:
                return "[UNKNOWN]";
        }
    }
};

#endif  // LOGGER_H