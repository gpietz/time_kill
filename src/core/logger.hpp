#pragma once

#include "prerequisites.hpp"
#include <fstream>
#include <mutex>

namespace time_kill::core {
    enum class LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    enum class DateFormat {
        DD_MM_YYYY,
        MM_DD_YYYY,
        YYYY_MM_DD
    };

    enum class DateSeparator {
        Hyphen,
        Period,
        Slash
    };

    class Logger {
    public:
        // Access via singleton
        static Logger& getInstance();

        // Initialize logging
        void init(const String& logFilePath, bool debugLoggingEnabled = false);

        // Logging methods
        void log(LogLevel level, const String& message);
        void trace(const String& message);
        void debug(const String& message);
        void info(const String& message);
        void warn(const String& message);
        void error(const String& message);

        // Debug logging
        void setDebugEnabled(bool enabled);
        [[nodiscard]] bool isDebugEnabled() const;

        // Trace logging
        void setTraceEnabled(bool enabled);
        [[nodiscard]] bool isTraceEnabled() const;

        // Dateformat
        void setDateFormat(DateFormat format);
        void setDateFormat(DateFormat format, DateSeparator separator);
        [[nodiscard]] DateFormat getDateFormat() const;

        // Date separator
        void setDateSeparator(DateSeparator separator);
        [[nodiscard]] DateSeparator getDateSeparator() const;

    private:
        Logger() = default; // Prevent instance creation
        ~Logger() = default;

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        // Auxiliary methods
        [[nodiscard]] String getTimestamp() const;
        static String levelToString(LogLevel level);

        std::ofstream logFile_;
        std::mutex logMutex_;
        bool debugLoggingEnabled_ = false;
        bool traceLoggingEnabled_ = false;
        DateFormat dateFormat_ = DateFormat::DD_MM_YYYY;
        DateSeparator dateSeparator_ = DateSeparator::Hyphen;
    };
}

namespace time_kill {
    inline void log_trace(const String& message) {
        core::Logger::getInstance().log(core::LogLevel::TRACE, message);
    }

    inline void log_debug(const String& message) {
        core::Logger::getInstance().log(core::LogLevel::DEBUG, message);
    }

    inline void log_info(const String& message) {
        core::Logger::getInstance().log(core::LogLevel::INFO, message);
    }

    inline void log_warn(const String& message) {
        core::Logger::getInstance().log(core::LogLevel::WARN, message);
    }

    inline void log_error(const String& message) {
        core::Logger::getInstance().log(core::LogLevel::ERROR, message);
    }

    inline void log_write(const core::LogLevel level, const String& message) {
        core::Logger::getInstance().log(level, message);
    }

    inline void log_enable_trace(const bool enabled) {
        core::Logger::getInstance().setTraceEnabled(enabled);
    }

    inline void log_enable_debug(const bool enabled) {
        core::Logger::getInstance().setDebugEnabled(enabled);
    }
}
