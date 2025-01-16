#ifndef UTILITIES_H
#define UTILITIES_H

#pragma once


// Logging levels
enum LogLevel {
    LOG_PRODUCTION = 0,  // Always shown
    LOG_DEBUG = 1,       // Shown in debug mode
    LOG_SCOPE = 2        // Most detailed, shown in scope mode
};

// Current logging level (can be changed at runtime)
extern LogLevel CURRENT_LOG_LEVEL;

// Debug logging function
void debugLog(LogLevel level, const char* format, ...);

// Logging macros for convenience
#define LOG_PROD(format, ...) debugLog(LOG_PRODUCTION, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) debugLog(LOG_DEBUG, format, ##__VA_ARGS__)
#define LOG_SCOPE(format, ...) debugLog(LOG_SCOPE, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) debugLog(LOG_PRODUCTION, "ERROR: " format, ##__VA_ARGS__)

#endif
