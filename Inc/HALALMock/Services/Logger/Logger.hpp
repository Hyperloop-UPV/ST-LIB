#include <string>

class Logger {
  public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        FATAL
    };
    enum class LogDest {
        Console = 0b10,
        File = 0b01
    };

    static void log(std::string, LogLevel level);
    static void debug(std::string msg);
    static void info(std::string msg);
    static void warning(std::string msg);
    static void error(std::string msg);
    static void fatal(std::string msg);

    static void setDest(LogDest dest);

  private:
    static int config;
    static void logToConsole(std::string msg);
    static void logToFile(std::string msg);
};