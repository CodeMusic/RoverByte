#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

namespace PrefrontalCortex 
{
    // Initialize static members
    Utilities::LogLevel Utilities::CURRENT_LOG_LEVEL = Utilities::LOG_LEVEL_DEBUG;
    
    const char* Utilities::LOG_LEVEL_STRINGS[] = 
    {
        "PROD",
        "ERROR", 
        "DEBUG",
        "INFO",
        "WARNING"
    };

    void Utilities::debugLog(LogLevel level, const char* format, ...) 
    {
        if (level <= CURRENT_LOG_LEVEL) 
        {
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            Serial.printf("[%s] %s\n", LOG_LEVEL_STRINGS[level], buffer);
        }
    }
}