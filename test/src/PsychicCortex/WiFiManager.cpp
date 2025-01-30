#include "WiFiManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include <WiFi.h>
#include <time.h>

namespace PsychicCortex 
{

    #define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
    #define TIME_CHECK_INTERVAL 500    // Time sync check interval
    #define RESYNC_TIME_INTERVAL 86400000 // 24h in ms


    bool WiFiManager::isWiFiConnected = false;
    unsigned long WiFiManager::lastWiFiAttempt = 0;
    bool WiFiManager::timeInitialized = false;

    bool WiFiManager::init() {
        bool success = false;
        WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
        
        if (WiFi.waitForConnectResult() == WL_CONNECTED) 
        {
            isWiFiConnected = true;
            success = true;
        } 
        else 
        {
            isWiFiConnected = false;
        }

        return success;
    }

    void WiFiManager::checkConnection() 
    {
        static unsigned long lastTimeCheck = 0;
        static int connectionAttempts = 0;
        static bool usingBackupNetwork = false;
        const unsigned long CHECK_INTERVAL = 500;

        if (!isWiFiConnected) 
        {
            if (millis() - lastTimeCheck >= CHECK_INTERVAL) 
            {
                lastTimeCheck = millis();
                
                if (WiFi.status() != WL_CONNECTED) 
                {
                    PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                        "WiFi Status: %d", WiFi.status());
                    
                    if (connectionAttempts == 0) 
                    {
                        WiFi.disconnect(true);
                        delay(100);
                        if (!usingBackupNetwork) 
                        {
                            PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                                "Attempting primary network...");
                            WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
                        } 
                        else 
                        {
                            PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_DEBUG, 
                                "Attempting backup network...");
                            WiFi.begin(BACKUP_SSID, BACKUP_PASSWORD);
                        }
                    }
                    
                    connectionAttempts++;
                    if (connectionAttempts >= 10) 
                    {
                        connectionAttempts = 0;
                        usingBackupNetwork = !usingBackupNetwork;
                    }
                } 
                else 
                {
                    isWiFiConnected = true;
                    connectionAttempts = 0;
                    PrefrontalCortex::Utilities::debugLog(
                        PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, 
                        "Network connection established"
                    );
                }
            }
        } 
        else if (!timeInitialized || millis() - lastTimeCheck >= RESYNC_TIME_INTERVAL) 
        { 
            syncTime();
        }
    }

    bool WiFiManager::syncTime() 
    {
        static unsigned long lastTimeCheck = 0;
        static int timeAttempts = 0;
        
        if (!timeInitialized && millis() - lastTimeCheck >= TIME_CHECK_INTERVAL) 
        {
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
            if (time(nullptr) > 1000000000) 
            {
                timeInitialized = true;
                PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, "Time sync complete");
                return true;
            } 
            else 
            {
                timeAttempts++;
                if (timeAttempts >= 40) 
                {
                    timeAttempts = 0;
                }
            }
            lastTimeCheck = millis();
        }
        return false;
    }

    bool WiFiManager::connectToWiFi() 
        {
        Serial.println("Starting WiFi connection process");
        WiFi.disconnect(true);  // Ensure clean connection attempt
        delay(100);
        WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
        lastWiFiAttempt = millis();
        isWiFiConnected = false;  // Reset connection state
        checkConnection();
        return isWiFiConnected;
    }
    // Generic error handler
    void handleError(const char* errorMessage) {
        PrefrontalCortex::Utilities::debugLog(PrefrontalCortex::Utilities::LOG_LEVEL_PRODUCTION, "Error: %s", errorMessage);
        VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::LOOKING_DOWN, 1000);
    }

    // Example usage in various scenarios
    void processAPIRequest(bool success = false) {  // Parameter with default value
        VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::LOOKING_UP);  // Looking up while thinking
        
        if (success) {
            VisualCortex::RoverManager::setTemporaryExpression(VisualCortex::RoverManager::BIG_SMILE, 1000);
        } else {
            handleError("API request failed");
        }
    }

}