#include "HALALMock/Services/Logger/Logger.hpp"
#include <ctime> // To manage time
#include <iostream> // To write in console
#include <fstream> // To write in a file
#include <memory> // To manage FileManager ptr

// === Class Logger ===

Logger::LogConf Logger::config = Logger::LogConf::Error
                                | Logger::LogConf::Fatal
                                | Logger::LogConf::Console
                                | Logger::LogConf::File;

std::unique_ptr<FileManager> Logger::file_manager = nullptr;

/// @brief Default log filename. To change it, use setFilename()
std::string Logger::log_filename = "STLIB.log";

void Logger::log(const std::string& msg, const LogLevel level) {

    // Check if this level is activated in config bitmask
    if(!hasFlag(config, level)) return;

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

    // Format message to include timestamp and Level
    // Reach timestamp
    time_t current_time = time(nullptr);
    std::string timestamp = ctime(&current_time);

    // Reach message to be printed
    std::string formatted_message = "[" + timestamp + "] " + "[" + log_level + "] " + msg;

    // Print message where has been configured to print
    if (hasFlag(config, LogConf::Console)) {  // Print msg into console
        std::cout << formatted_message;
    }

    if (hasFlag(config, LogConf::File)) {  // Write message into a file
        file_manager->add(formatted_message);
    }
}

void Logger::debug(const std::string& msg) {
    log(msg, LogLevel::DEBUG);
}

void Logger::info(const std::string& msg) {
    log(msg, LogLevel::INFO);
}

void Logger::warning(const std::string& msg) {
    log(msg, LogLevel::WARNING);
}

void Logger::error(const std::string& msg) {
    log(msg, LogLevel::ERROR);
}

void Logger::fatal(const std::string& msg) {
    log(msg, LogLevel::FATAL);
}

/// @brief Sets the Logger configuration
/// @param conf Configuration of the Logger. It must be a bit mask of LogConf values, using '|' operator.
void Logger::setConf(const LogConf conf) {
    config = conf;
    if (hasFlag(conf, LogConf::File)) {
        if (hasFlag(conf, LogConf::DestructiveLog)) {
            Logger::file_manager = std::make_unique<FileManager>(std::ios_base::trunc);
        } else {
            Logger::file_manager = std::make_unique<FileManager>(std::ios_base::app);
        }
    } 
}

void Logger::setFilename(const std::string& filename) {
        log_filename = filename;
    }

// === Class FileManager ===

FileManager::FileManager(const std::ios_base::openmode file_mode) {
    file_stream = std::ofstream(Logger::log_filename, file_mode);
    if (!file_stream.is_open()) {
        std::cerr << "LOGGER ERROR. Couldn't open file " << Logger::log_filename << ", writing logs in STLIB.log" << std::endl;
        file_stream = std::ofstream("STLIB.log", file_mode);
    }
}

void FileManager::add(const std::string& msg) {
    file_stream << msg << std::endl;
}

FileManager::~FileManager() {
    file_stream.close();
}