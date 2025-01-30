#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using PC::SystemTypes::LogLevel;  // More specific using statement

    // Initialize static members
    LogLevel Utilities::CURRENT_LOG_LEVEL = LogLevel::DEBUG;
    
    const char* Utilities::LOG_LEVEL_STRINGS[] = 
    {
        "PROD",
        "ERROR", 
        "DEBUG",
        "INFO",
        "WARNING"
    };

    // Private helper function implementation
    void Utilities::debugLog(LogLevel level, const char* format, va_list args) 
    {
        if (level <= CURRENT_LOG_LEVEL) 
        {
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.printf("[%s] %s\n", LOG_LEVEL_STRINGS[static_cast<int>(level)], buffer);
        }
    }

    // Public logging function implementations
    void Utilities::LOG_PROD(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::PRODUCTION, format, args);
        va_end(args);
    }

    void Utilities::LOG_ERROR(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::ERROR, format, args);
        va_end(args);
    }

    void Utilities::LOG_DEBUG(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::DEBUG, format, args);
        va_end(args);
    }

    void Utilities::LOG_INFO(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::INFO, format, args);
        va_end(args);
    }

    void Utilities::LOG_WARNING(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::WARNING, format, args);
        va_end(args);
    }
}