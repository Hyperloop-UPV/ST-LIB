#include "HALALMock/Services/Logger/Logger.hpp"
#include <ctime>
#include <iostream>

void Logger::log(std::string msg, LogLevel level) {
    // Format message to include timestamp and Level
    // Reach timestamp
    time_t current_time = time(nullptr);
    std::string timestamp = ctime(&current_time);

    // Reach Log Level
    std::string log_level;
    switch (level) {
        case LogLevel::DEBUG:
            log_level = "DEBUG";
        case LogLevel::INFO:
            log_level = "INFO";
        case LogLevel::WARNING:
            log_level = "WARNING";
        case LogLevel::ERROR:
            log_level = "ERROR";
        case LogLevel::FATAL:
            log_level = "FATAL";
    }

    // Reach message to be printed
    std::string formatted_message = "[" + timestamp + "] " + "[" + log_level + "] " + msg + "\n";

    // Print message where has been configured to print
    if (config & static_cast<int>(LogDest::Console)) {  // Print msg into console
        std::cout << formatted_message;
    }
}

void Logger::debug(std::string msg) { log(msg, LogLevel::DEBUG); }

void Logger::info(std::string msg) { log(msg, LogLevel::INFO); }

void Logger::warning(std::string msg) { log(msg, LogLevel::WARNING); }

void Logger::error(std::string msg) { log(msg, LogLevel::ERROR); }

void Logger::fatal(std::string msg) { log(msg, LogLevel::FATAL); }

void Logger::setDest(LogDest dest) { config = config | static_cast<int>(dest); }

void Logger::logToConsole(std::string msg) {}

void Logger::logToFile(std::string msg) {}
