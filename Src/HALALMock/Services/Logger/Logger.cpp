#include "HALALMock/Services/Logger/Logger.hpp"
#include <ctime> // To manage time
#include <iostream> // To write in console
#include <fstream> // To write in a file
#include <memory> // To manage FileManager ptr

// === Class Logger ===
std::unique_ptr<FileManager> Logger::file_manager = std::make_unique<FileManager>(std::ios_base::trunc);
int Logger::line = 0;
const char *Logger::function = nullptr;
const char *Logger::file = nullptr;

void Logger::log(const std::string& msg, const LogLevel level, const char *colour) {

    // Check if this level is activated in config bitmask
    if(!hasFlag(Log::config, level)) return;

    // Reach Log Level
    std::string log_level;
    switch (level) {
        case LogLevel::DEBUG:
            log_level = "DEBUG";
            break;
        case LogLevel::INFO:
            log_level = "INFO";
            break;
        case LogLevel::WARNING:
            log_level = "WARNING";
            break;
        case LogLevel::ERROR:
            log_level = "ERROR";
            break;
        case LogLevel::FATAL:
            log_level = "FATAL";
            break;
    }

    // Format message to include timestamp and Level
    // Reach timestamp
    time_t current_time = time(nullptr);
    char timestamp[19];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %T", localtime(&current_time));

    // Reach message to be printed
    std::string formatted_message = "[" + std::string(timestamp) + "] " +
                                    "[" + colour + log_level + ANSI_COLOR_RESET +
                                    "] " + "[" + file + ":" + std::to_string(line) +
                                    " in " + function + "] " + msg;

    // Print message where has been configured to print
    if (hasFlag(Log::config, LogConf::Console)) {  // Print msg into console
        std::cout << formatted_message << std::endl;
    }

    if (hasFlag(Log::config, LogConf::File)) {  // Write message into a file
        file_manager->add(formatted_message);
    }
}

void Logger::set_metadata(int line, const char *function, const char *file) {
    Logger::line = line;
    Logger::function = function;
    Logger::file = file;
}

// === Class FileManager ===

FileManager::FileManager(const std::ios_base::openmode file_mode) {
    file_stream = std::ofstream(Log::filename, file_mode);
    if (!file_stream.is_open()) {
        std::cerr << "LOGGER ERROR. Couldn't open file " << Log::filename << ", writing logs in STLIB.log" << std::endl;
        file_stream = std::ofstream("STLIB.log", file_mode);
    }
}

void FileManager::add(const std::string& msg) {
    file_stream << msg << std::endl;
}

FileManager::~FileManager() {
    file_stream.close();
}