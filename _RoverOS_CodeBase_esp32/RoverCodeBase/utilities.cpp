#include "utilities.h"
#include <Arduino.h>
#include <stdarg.h>

// Initialize to production logging by default
LogLevel CURRENT_LOG_LEVEL = LOG_PRODUCTION;

void debugLog(LogLevel level, const char* format, ...) {
    // Check if we should log this message
    if (level > CURRENT_LOG_LEVEL) {
        return;
    }

    // Buffer for timestamp
    char timestamp[16];
    unsigned long ms = millis();
    sprintf(timestamp, "[%lu.%03lu] ", ms/1000, ms%1000);
    Serial.print(timestamp);

    // Add log level prefix
    switch (level) {
        case LOG_PRODUCTION:
            Serial.print("[PROD] ");
            break;
        case LOG_DEBUG:
            Serial.print("[DEBUG] ");
            break;
        case LOG_SCOPE:
            Serial.print("[SCOPE] ");
            break;
    }

    // Format and print the actual message
    char temp[256];
    va_list args;
    va_start(args, format);
    vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);
    
    Serial.println(temp);
} 