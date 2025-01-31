#ifndef UTILITIES_H
#define UTILITIES_H

#include "../PrefrontalCortex/ProtoPerceptions.h"  // Direct include for LogLevel
#include "../../RoverConfig.h"  // For default log level

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;
    using PC::SystemTypes::LogLevel;  // Only import what we need

    class Utilities 
    {
    public:
        // Use SystemTypes::LogLevel from ProtoPerceptions
        static LogLevel CURRENT_LOG_LEVEL;  // Will be initialized from RoverConfig

        // Logging functions with proper variadic arguments
        static void LOG_PROD(const char* format, ...);
        static void LOG_ERROR(const char* format, ...);
        static void LOG_DEBUG(const char* format, ...);
        static void LOG_INFO(const char* format, ...);
        static void LOG_WARNING(const char* format, ...);
        static void LOG_SCOPE(const char* format, ...);

    private:
        static void debugLog(LogLevel level, const char* format, va_list args);
    };
}

#endif // UTILITIES_H
