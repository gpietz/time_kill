#include "logger.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#ifdef DEBUG_LOGGING_ENABLED
#define DEBUG_LOG(message) Logger::getInstance().log(LogLevel::DEBUG, message)
#else
#define DEBUG_LOG(message)
#endif

namespace time_kill::core {
    Logger& Logger::getInstance() {
        static Logger instance;
        return instance;
    }

    void Logger::init(const String &logFilePath, const bool debugLoggingEnabled) {
        std::lock_guard<std::mutex> lock(logMutex_);
        if (logFile_.is_open()) {
            logFile_.close();
        }

        logFile_.open(logFilePath, std::fstream::out | std::fstream::app);
        if (!logFile_.is_open()) {
            throw std::runtime_error("Failed to open log file: " + logFilePath);
        }

        debugLoggingEnabled_ = debugLoggingEnabled;
    }

    void Logger::log(const LogLevel level, const String &message) {
        if (level == LogLevel::DEBUG && !debugLoggingEnabled_) {
            return;
        }

        std::lock_guard<std::mutex> lock(logMutex_);
        const auto logMessage = getTimestamp() + " [" + levelToString(level) + "] " + message;

        // Output to console
        std::cout << logMessage << "\n";

        // Output to file
        if (logFile_.is_open()) {
            logFile_ << logMessage << "\n";
        }
    }

    void Logger::debug(const String &message) {
        log(LogLevel::DEBUG, message);
    }

    void Logger::info(const String &message) {
        log(LogLevel::INFO, message);
    }

    void Logger::warn(const String &message) {
        log(LogLevel::WARN, message);
    }

    void Logger::error(const String &message) {
        log(LogLevel::ERROR, message);
    }

    void Logger::setDebugEnabled(const bool enabled) {
        debugLoggingEnabled_ = enabled;
    }

    bool Logger::isDebugEnabled() const {
        return debugLoggingEnabled_;
    }

    void Logger::setDateFormat(const DateFormat format) {
        dateFormat_ = format;
    }

    void Logger::setDateFormat(const DateFormat format, const DateSeparator separator) {
        dateFormat_ = format;
        dateSeparator_ = separator;
    }

    DateFormat Logger::getDateFormat() const {
        return dateFormat_;
    }

    void Logger::setDateSeparator(const DateSeparator separator) {
        dateSeparator_ = separator;
    }

    DateSeparator Logger::getDateSeparator() const {
        return dateSeparator_;
    }

    constexpr std::string_view dateSeparatorToString(const DateSeparator sep) {
        switch (sep) {
            case DateSeparator::Hyphen: return "-";
            case DateSeparator::Period: return ".";
            case DateSeparator::Slash: return "/";
            default: return "";
        }
    }

    String formatDateTime(const std::chrono::system_clock::time_point now,
        const DateFormat format,
        const DateSeparator separator
    ) {
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = {};
        localtime_s(&tm, &time);

        const std::string_view sep = dateSeparatorToString(separator);

        std::ostringstream oss;
        switch (format) {
            case DateFormat::DD_MM_YYYY:
                oss << std::put_time(&tm, ("%d" + std::string(sep) + "%m" + std::string(sep) + "%Y %H:%M:%S").c_str());
            break;
            case DateFormat::MM_DD_YYYY:
                oss << std::put_time(&tm, ("%m" + std::string(sep) + "%d" + std::string(sep) + "%Y %H:%M:%S").c_str());
            break;
            case DateFormat::YYYY_MM_DD:
                oss << std::put_time(&tm, ("%Y" + std::string(sep) + "%m" + std::string(sep) + "%d %H:%M:%S").c_str());
            break;
            default:
                return "";
        }

        return oss.str();
    }

    String Logger::getTimestamp() const {
        using namespace std::chrono;
        const auto now = system_clock::now();
        const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << formatDateTime(now, dateFormat_, dateSeparator_);
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    String Logger::levelToString(const LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            default:              return "?????";
        }
    }
}
