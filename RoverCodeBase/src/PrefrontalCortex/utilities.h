#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>
#include <stdarg.h>

namespace PrefrontalCortex 
{
    class Utilities 
    {
    public:
        // Logging levels
        enum LogLevel 
        {
            LOG_LEVEL_PRODUCTION = 0,  // Always shown
            LOG_LEVEL_ERROR = 1,       // Error level
            LOG_LEVEL_DEBUG = 2,       // Debug level
            LOG_LEVEL_INFO = 3,        // Info level
            LOG_LEVEL_WARNING = 4      // Warning level
        };

        // Current logging level (can be changed at runtime)
        static LogLevel CURRENT_LOG_LEVEL;
        static const char* LOG_LEVEL_STRINGS[];

        // Core logging functions
        static void debugLog(LogLevel level, const char* format, ...);
        static void errorLog(const char* format, ...);
        
        // Helper logging functions (inline)
        static inline void logProduction(const char* format, ...) 
        {
            va_list args;
            va_start(args, format);
            debugLog(LOG_LEVEL_PRODUCTION, format);
            va_end(args);
        }
        
        static inline void logError(const char* format, ...) 
        {
            va_list args;
            va_start(args, format);
            debugLog(LOG_LEVEL_ERROR, format);
            va_end(args);
        }
        
        static inline void logDebug(const char* format, ...) 
        {
            va_list args;
            va_start(args, format);
            debugLog(LOG_LEVEL_DEBUG, format);
            va_end(args);
        }
        
        static void logInfo(const char* format, ...);
        static void logWarning(const char* format, ...);
        
        static const char* logLevelToString(LogLevel level);
    };
}

// Convenience macros for logging
#define LOG_PROD(...) Utilities::logProduction(__VA_ARGS__)
#define LOG_ERROR(...) Utilities::logError(__VA_ARGS__)
#define LOG_DEBUG(...) Utilities::logDebug(__VA_ARGS__)
#define LOG_INFO(...) Utilities::logInfo(__VA_ARGS__)
#define LOG_WARNING(...) Utilities::logWarning(__VA_ARGS__)

#endif // UTILITIES_H
