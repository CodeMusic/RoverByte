#ifndef UTILITIES_H
#define UTILITIES_H
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using namespace SystemTypes;
    using namespace StorageTypes;
    
    class Utilities 
    {
    public:
        // Use SystemTypes::LogLevel from ProtoPerceptions
        static SystemTypes::LogLevel CURRENT_LOG_LEVEL;
        static const char* LOG_LEVEL_STRINGS[];

        // Logging macros for convenience
        static void LOG_PROD(const char* format, ...) { debugLog(SystemTypes::LogLevel::PRODUCTION, format); }
        static void LOG_ERROR(const char* format, ...) { debugLog(SystemTypes::LogLevel::ERROR, format); }
        static void LOG_DEBUG(const char* format, ...) { debugLog(SystemTypes::LogLevel::DEBUG, format); }
        static void LOG_INFO(const char* format, ...) { debugLog(SystemTypes::LogLevel::INFO, format); }
        static void LOG_WARNING(const char* format, ...) { debugLog(SystemTypes::LogLevel::WARNING, format); }

    private:
        static void debugLog(SystemTypes::LogLevel level, const char* format, ...);
    };
}

#endif // UTILITIES_H
