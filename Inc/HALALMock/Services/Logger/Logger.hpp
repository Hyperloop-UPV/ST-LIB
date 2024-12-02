#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <memory>
#include <fstream>

class FileManager {
  public:
    FileManager(const std::ios_base::openmode file_mode);
    void add(const std::string& msg);
    ~FileManager();

  private:
    std::ofstream file_stream;

};

class Logger {
  public:
    enum class LogLevel : unsigned int {
        DEBUG   = 0b00001,
        INFO    = 0b00010,
        WARNING = 0b00100,
        ERROR   = 0b01000,
        FATAL   = 0b10000,
    };

    /// @brief Bitmask that represents log configuration
    enum class LogConf : unsigned int {
        /// @brief Includes Debug messages in logs
        Debug           = 0b00000001,
        /// @brief Includes Info messages in logs
        Info            = 0b00000010,
        /// @brief Includes Warning messages in logs
        Warning         = 0b00000100,
        /// @brief Includes Error messages in logs
        Error           = 0b00001000,
        /// @brief Includes Fatal messages in logs
        Fatal           = 0b00010000,
        /// @brief Writes logs into the console
        Console         = 0b00100000,
        /// @brief Writes logs into a file, defined in filename_log external variable
        File            = 0b01000000,
        /// @brief Erase previous logs
        DestructiveLog  = 0b10000000
    };

    static void log(const std::string& msg, const LogLevel level);
    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warning(const std::string& msg);
    static void error(const std::string& msg);
    static void fatal(const std::string& msg);

    static void setConf(const LogConf conf);
    static void setFilename(const std::string& filename);

    friend class FileManager;

  private:
    static LogConf config;
    static std::unique_ptr<FileManager> file_manager;
    static std::string log_filename;
};

// Overload bit operators for LogConf enum
constexpr Logger::LogConf operator|(const Logger::LogConf& conf1, const Logger::LogConf& conf2) {
    return static_cast<Logger::LogConf>(static_cast<int>(conf1) | static_cast<int>(conf2));
};
constexpr Logger::LogConf operator&(const Logger::LogConf& conf, const Logger::LogLevel& level) {
    return static_cast<Logger::LogConf>(static_cast<int>(conf) & static_cast<int>(level));
};
constexpr Logger::LogConf operator&(const Logger::LogConf& conf1, const Logger::LogConf& conf2) {
    return static_cast<Logger::LogConf>(static_cast<int>(conf1) & static_cast<int>(conf2));
};

/// @brief Check if a flag is activated in a mask
/// @param mask Mask where you want to check the flag
/// @param flag Flag which you want to check in the mask
/// @return If the flag is activated in the mask
constexpr bool hasFlag(Logger::LogConf mask, Logger::LogConf flag) {
    return (mask & flag) == flag;
};
constexpr bool hasFlag(Logger::LogConf mask, Logger::LogLevel flag) {
    return (mask & flag) == static_cast<Logger::LogConf>(flag);
};

#endif