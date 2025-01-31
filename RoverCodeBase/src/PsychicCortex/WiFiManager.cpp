#include "WiFiManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../MotorCortex/PinDefinitions.h"
#include <WiFi.h>
#include <time.h>

namespace PsychicCortex 
{
    namespace PSY = PsychicCortex;  // Add namespace alias
    using namespace CorpusCallosum;
    using PC::Utilities;
    using PC::NetworkTypes::NetworkCredentials;

    #define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
    #define TIME_CHECK_INTERVAL 500    // Time sync check interval
    #define RESYNC_TIME_INTERVAL 86400000 // 24h in ms

    // Initialize static members
    bool WiFiManager::isWiFiConnected = false;
    unsigned long WiFiManager::lastWiFiAttempt = 0;
    bool WiFiManager::timeInitialized = false;
    unsigned long WiFiManager::lastTimeCheck = 0;

    bool WiFiManager::init() 
    {
        bool success = false;
        WiFi.begin(PRIMARY_NETWORK.ssid, PRIMARY_NETWORK.password);
        
        if (WiFi.waitForConnectResult() == WL_CONNECTED) 
        {
            isWiFiConnected = true;
            success = true;
            Utilities::LOG_PROD("Connected to primary network");
        } 
        else 
        {
            isWiFiConnected = false;
            Utilities::LOG_DEBUG("Primary network connection failed");
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
                    Utilities::LOG_DEBUG("WiFi Status: %d", WiFi.status());
                    
                    if (connectionAttempts == 0) 
                    {
                        WiFi.disconnect(true);
                        delay(100);
                        if (!usingBackupNetwork) 
                        {
                            Utilities::LOG_DEBUG("Attempting primary network...");
                            WiFi.begin(PRIMARY_NETWORK.ssid, PRIMARY_NETWORK.password);
                        } 
                        else 
                        {
                            Utilities::LOG_DEBUG("Attempting backup network...");
                            WiFi.begin(BACKUP_NETWORK.ssid, BACKUP_NETWORK.password);
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
                    Utilities::LOG_PROD("Network connection established");
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
                Utilities::LOG_PROD("Time sync complete");
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
        Utilities::LOG_DEBUG("Starting WiFi connection process");
        WiFi.disconnect(true);  // Ensure clean connection attempt
        delay(100);
        WiFi.begin(PRIMARY_NETWORK.ssid, PRIMARY_NETWORK.password);
        lastWiFiAttempt = millis();
        isWiFiConnected = false;  // Reset connection state
        checkConnection();
        return isWiFiConnected;
    }

    // Generic error handler
    void handleError(const char* errorMessage) {
        PC::Utilities::LOG_PROD(errorMessage);
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