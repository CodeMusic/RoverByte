#include "Utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>
#include "../RoverConfig.h"

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using PC::SystemTypes::LogLevel;  // More specific using statement

    // Initialize with config value
    LogLevel Utilities::CURRENT_LOG_LEVEL = RoverConfig::DEFAULT_LOG_LEVEL;
    
    // Private helper function implementation
    void Utilities::debugLog(LogLevel level, const char* format, va_list args) 
    {
        if (level <= CURRENT_LOG_LEVEL) 
        {
            char buffer[256];
            vsnprintf(buffer, sizeof(buffer), format, args);
            Serial.printf("[%s] %s\n", PC::SystemTypes::LOG_LEVEL_STRINGS[static_cast<int>(level)], buffer);
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

    /**
     * @brief Debug level logging for development and troubleshooting
     * Outputs detailed information about system state and operations
     */
    void Utilities::LOG_DEBUG(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::DEBUG, format, args);
        va_end(args);
    }

    void Utilities::LOG_SCOPE(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::SCOPE, format, args);
        va_end(args);
    }

    /**
     * @brief Information level logging for normal operations
     * Outputs general status updates and non-critical information
     */
    void Utilities::LOG_INFO(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::INFO, format, args);
        va_end(args);
    }

    /**
     * @brief Warning level logging for potential issues
     * Outputs messages about concerning but non-critical situations
     */
    void Utilities::LOG_WARNING(const char* format, ...) 
    {
        va_list args;
        va_start(args, format);
        debugLog(LogLevel::WARNING, format, args);
        va_end(args);
    }
}