#pragma once

#include <stdio.h>

#include <fstream>
#include <memory>
#include <string>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_GREY "\x1b[37m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define LOG_DEBUG(x)                                            \
    do {                                                        \
        Logger::set_metadata(__LINE__, __FUNCTION__, __FILE__); \
        if (hasFlag(Log::config, Logger::LogLevel::DEBUG))      \
            Logger::__log(x, "DEBUG", ANSI_COLOR_GREY);           \
    } while (0)

#define LOG_INFO(x)                                             \
    do {                                                        \
        Logger::set_metadata(__LINE__, __FUNCTION__, __FILE__); \
        if (hasFlag(Log::config, Logger::LogLevel::INFO))       \
            Logger::__log(x, "INFO", ANSI_COLOR_GREEN);           \
    } while (0)

#define LOG_WARNING(x)                                          \
    do {                                                        \
        Logger::set_metadata(__LINE__, __FUNCTION__, __FILE__); \
        if (hasFlag(Log::config, Logger::LogLevel::WARNING))    \
            Logger::__log(x, "WARNING", ANSI_COLOR_YELLOW);       \
    } while (0)

#define LOG_ERROR(x)                                            \
    do {                                                        \
        Logger::set_metadata(__LINE__, __FUNCTION__, __FILE__); \
        if (hasFlag(Log::config, Logger::LogLevel::ERROR))      \
            Logger::__log(x, "ERROR", ANSI_COLOR_RED);            \
    } while (0)

#define LOG_FATAL(x)                                            \
    do {                                                        \
        Logger::set_metadata(__LINE__, __FUNCTION__, __FILE__); \
        if (hasFlag(Log::config, Logger::LogLevel::FATAL))      \
            Logger::__log(x, "FATAL", ANSI_COLOR_MAGENTA);        \
    } while (0)

class FileManager {
   public:
    FileManager(const std::ios_base::openmode file_mode);
    void add(const std::string& msg);
    ~FileManager();

   private:
    std::ofstream file_stream;
};

/** @brief Class used to Log info in simulator mode.
 * You can log in five levels: DEBUG, INFO, WARNING, ERROR and FATAL.
 *
 * To configure it, you must set in the extern variable config
 * a mask 'oring' the bits that represents what you want.
 * You can configure which level of logs you want (DEBUG, INFO...),
 * if you want to write logs on the console and/or on a file,
 * and if you want to erase or not the log file.
 *
 * To use it, you must use one of the five MACROS defined in this file,
 * each one representing the level of the log you want to print.
 *
 * As a tip, you can use std::format to log formatted messages
 */
class Logger {
   public:
    enum class LogLevel : unsigned int {
        DEBUG = 0b00001,
        INFO = 0b00010,
        WARNING = 0b00100,
        ERROR = 0b01000,
        FATAL = 0b10000,
    };

    static void __log(const std::string& msg, const std::string& level,
                    const char* colour);
    static void set_metadata(int line, const char* function, const char* file);

   private:
    static std::unique_ptr<FileManager> file_manager;
    static int line;
    static const char* function;
    static const char* file;
};

namespace Log {
extern std::string filename;
/// @brief Bitmask that represents log configuration
enum class LogConf : unsigned int {
    /// @brief Includes Debug messages in logs
    Debug = static_cast<int>(Logger::LogLevel::DEBUG),
    /// @brief Includes Info messages in logs
    Info = static_cast<int>(Logger::LogLevel::INFO),
    /// @brief Includes Warning messages in logs
    Warning = static_cast<int>(Logger::LogLevel::WARNING),
    /// @brief Includes Error messages in logs
    Error = static_cast<int>(Logger::LogLevel::ERROR),
    /// @brief Includes Fatal messages in logs
    Fatal = static_cast<int>(Logger::LogLevel::FATAL),
    /// @brief Writes logs into the console
    Console = 0b00100000,
    /// @brief Writes logs into a file, defined in filename_log external
    /// variable
    File = 0b01000000,
    /// @brief Erase previous logs
    DestructiveLog = 0b10000000
};
extern LogConf config;
}  // namespace Log

using namespace Log;
// Overload bit operators for LogConf enum
constexpr LogConf operator|(const LogConf& conf1, const LogConf& conf2) {
    return static_cast<LogConf>(static_cast<int>(conf1) |
                                static_cast<int>(conf2));
};
constexpr LogConf operator&(const LogConf& conf,
                            const Logger::LogLevel& level) {
    return static_cast<LogConf>(static_cast<int>(conf) &
                                static_cast<int>(level));
};
constexpr LogConf operator&(const LogConf& conf1, const LogConf& conf2) {
    return static_cast<LogConf>(static_cast<int>(conf1) &
                                static_cast<int>(conf2));
};

/// @brief Check if a flag is activated in a mask
/// @param mask Mask where you want to check the flag
/// @param flag Flag which you want to check in the mask
/// @return If the flag is activated in the mask
constexpr bool hasFlag(const LogConf mask, const LogConf flag) {
    return (mask & flag) == flag;
};
constexpr bool hasFlag(const LogConf mask, const Logger::LogLevel flag) {
    return (mask & flag) == static_cast<LogConf>(flag);
};
