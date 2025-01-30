#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using namespace SystemTypes;
    using namespace StorageTypes;
    
    // Initialize static members
    SystemTypes::LogLevel Utilities::CURRENT_LOG_LEVEL = SystemTypes::LogLevel::DEBUG;
    
    const char* Utilities::LOG_LEVEL_STRINGS[] = 
    {
        "PROD",
        "ERROR", 
        "DEBUG",
        "INFO",
        "WARNING"
    };

    void Utilities::debugLog(SystemTypes::LogLevel level, const char* format, ...) 
    {
        if (level <= CURRENT_LOG_LEVEL) 
        {
            char buffer[256];
            va_list args;
            va_start(args, format);
            vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
            
            Serial.printf("[%s] %s\n", LOG_LEVEL_STRINGS[static_cast<int>(level)], buffer);
        }
    }
}