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
            String logLevelString = PC::SystemTypes::LOG_LEVEL_STRINGS[static_cast<int>(level)];
            Serial.printf("[%s] %s\n", logLevelString.c_str(), buffer);
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

    // Add new helper for parameter formatting
    String Utilities::formatParameter(const String& type, const String& value) 
    {
        // If value is a string (starts and ends with quotes), return as is
        if (value.startsWith("\"") || value.startsWith("'")) {
            return value;
        }
        // If value is numeric, return as is
        if (isdigit(value[0]) || value[0] == '-' || value[0] == '.') {
            return value;
        }
        // Otherwise treat as complex type
        return "<" + type + ">";
    }

    void Utilities::LOG_SCOPE(const char* scope, ...) 
    {
        if (CURRENT_LOG_LEVEL >= LogLevel::SCOPE) 
        {
            va_list args;
            va_start(args, scope);
            
            // Parse function name and parameters from scope string
            String scopeStr = scope;
            int paramStart = scopeStr.indexOf('(');
            
            if (paramStart == -1) {
                // No parameters
                Serial.printf("[SCOPE] %s()\n", scope);
            } else {
                String funcName = scopeStr.substring(0, paramStart);
                String params = scopeStr.substring(paramStart + 1, scopeStr.length() - 1);
                
                // Split parameters
                String formattedParams = "";
                int paramCount = 0;
                
                while (params.length() > 0) {
                    int comma = params.indexOf(',');
                    String param;
                    
                    if (comma == -1) {
                        param = params;
                        params = "";
                    } else {
                        param = params.substring(0, comma);
                        params = params.substring(comma + 1);
                    }
                    
                    // Get parameter value from va_args
                    String value = va_arg(args, const char*);
                    String type = param.substring(0, param.indexOf(' '));
                    
                    if (paramCount > 0) formattedParams += ",";
                    formattedParams += formatParameter(type, value);
                    paramCount++;
                }
                
                Serial.printf("[SCOPE] <@%d> %s(%s)\n", millis(), funcName.c_str(), formattedParams.c_str());
            }
            
            va_end(args);
        }
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