#include "utils/Logger.hpp"
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>

namespace clustering {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void Logger::setLevel(LogLevel level) {
    m_level = level;
}

void Logger::setOutputFile(const std::string& filename) {
    if (m_file.is_open()) {
        m_file.close();
    }
    m_file.open(filename, std::ios::app);
}

void Logger::setConsoleOutput(bool enabled) {
    m_consoleOutput = enabled;
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_level) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << "[" << getTimestamp() << "] "
       << "[" << levelToString(level) << "] "
       << message;
    
    if (m_consoleOutput) {
        if (level == LogLevel::ERROR) {
            std::cerr << ss.str() << std::endl;
        } else {
            std::cout << ss.str() << std::endl;
        }
    }
    
    if (m_file.is_open()) {
        m_file << ss.str() << std::endl;
    }
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::NONE:    return "NONE";
        default:                return "UNKNOWN";
    }
}

} // namespace clustering