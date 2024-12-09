#include "HALALMock/Services/Logger/Logger.hpp"

#include <ctime>     // To manage time
#include <fstream>   // To write in a file
#include <iostream>  // To write in console
#include <memory>    // To manage FileManager ptr

// === Class Logger ===
std::unique_ptr<FileManager> Logger::file_manager =
    std::make_unique<FileManager>(std::ios_base::trunc);
int Logger::line = 0;
const char *Logger::function = nullptr;
const char *Logger::file = nullptr;

void Logger::log(const std::string &msg, const std::string &level,
                 const char *colour) {

    // Format message to include timestamp and Level
    // Reach timestamp
    time_t current_time = time(nullptr);
    char timestamp[19];
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %T",
             localtime(&current_time));

    // Get message to be printed
    std::string formatted_message =
        "[" + std::string(timestamp) + "] " + "[" + colour + level +
        ANSI_COLOR_RESET + "] " + "[" + file + ":" + std::to_string(line) +
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
        std::cerr << "LOGGER ERROR. Couldn't open file " << Log::filename
                  << ", writing logs in STLIB.log" << std::endl;
        file_stream = std::ofstream("STLIB.log", file_mode);
    }
}

void FileManager::add(const std::string &msg) {
    file_stream << msg << std::endl;
}

FileManager::~FileManager() { file_stream.close(); }