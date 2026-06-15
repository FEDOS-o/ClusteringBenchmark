#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

namespace clustering {

    enum class LogLevel {
        DEBUG, INFO, WARNING, ERROR, NONE
    };

    class Logger {
    public:
        static Logger& getInstance();
        void setLevel(LogLevel level);
        void setOutputFile(const std::string& filename);
        void setConsoleOutput(bool enabled);

        void debug(const std::string& message);
        void info(const std::string& message);
        void warning(const std::string& message);
        void error(const std::string& message);

        std::string getTimestamp() const;

    private:
        Logger() = default;
        ~Logger();
        void log(LogLevel level, const std::string& message);
        std::string levelToString(LogLevel level) const;

        LogLevel m_level = LogLevel::INFO;
        std::ofstream m_file;
        bool m_consoleOutput = true;
        std::mutex m_mutex;
    };

#define LOG_DEBUG(msg) clustering::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) clustering::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) clustering::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) clustering::Logger::getInstance().error(msg)

} // namespace clustering