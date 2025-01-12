#include "utilities.h"
#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>

// Initialize the logging level
LogLevel CURRENT_LOG_LEVEL = LOG_PRODUCTION;

void debugLog(LogLevel level, const char* format, ...) {
    // Only print if the current log level is high enough
    if (level <= CURRENT_LOG_LEVEL) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        // Print the formatted string
        Serial.println(buffer);
    }
} 