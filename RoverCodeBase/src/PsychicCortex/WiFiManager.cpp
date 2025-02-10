/**
 * @brief WiFiManager implementation
 * 
 * Handles:
 * - Network connection establishment
 * - Connection state management
 * - Status updates and feedback
 * - Error handling and recovery
 * - Integration with visual and audio systems
 */

#include "WiFiManager.h"
#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../../RoverConfig.h"
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
    size_t WiFiManager::currentNetwork = 0;
    uint8_t WiFiManager::connectionAttempts = 0;
    uint8_t WiFiManager::totalAttempts = 0;
    constexpr const NetworkCredentials WiFiManager::AVAILABLE_NETWORKS[];

    /**
     * @brief Initialize WiFi hardware and prepare for connections
     * Sets up WiFi hardware, configures modes, and prepares status indicators
     */
    bool WiFiManager::init() 
    {
        Utilities::LOG_SCOPE("WiFiManager::init()");
        bool success = false;
        static size_t currentNetwork = 0;
        
        WiFi.disconnect(true);
        delay(100);
        
        WiFi.begin(AVAILABLE_NETWORKS[currentNetwork].ssid, 
                   AVAILABLE_NETWORKS[currentNetwork].password);
        
        if (WiFi.waitForConnectResult() == WL_CONNECTED) 
        {
            isWiFiConnected = true;
            success = true;
            Utilities::LOG_PROD("Connected to network: %s", AVAILABLE_NETWORKS[currentNetwork].ssid);
        } 
        else 
        {
            isWiFiConnected = false;
            currentNetwork = (currentNetwork + 1) % NETWORK_COUNT;
            Utilities::LOG_DEBUG("Network connection failed, will try next network");
        }

        return success;
    }

    void WiFiManager::update() 
    {
        Utilities::LOG_SCOPE("WiFiManager::update()");
        static size_t currentNetwork = 0;
        static uint8_t totalAttempts = 0;
        static const uint8_t MAX_TOTAL_ATTEMPTS = ROVER_WIFI_MAX_ATTEMPTS;
        
        if (!isWiFiConnected && totalAttempts < MAX_TOTAL_ATTEMPTS) 
        {
            if (millis() - lastTimeCheck >= TIME_CHECK_INTERVAL) 
            {
                lastTimeCheck = millis();
                
                if (WiFi.status() != WL_CONNECTED) 
                {
                    if (connectionAttempts == 0) 
                    {
                        WiFi.disconnect(true);
                        delay(100);
                        
                        Utilities::LOG_DEBUG("Attempting network: %s (Attempt %d/%d)", 
                            AVAILABLE_NETWORKS[currentNetwork].ssid,
                            totalAttempts + 1,
                            MAX_TOTAL_ATTEMPTS);
                            
                        WiFi.begin(AVAILABLE_NETWORKS[currentNetwork].ssid, 
                                 AVAILABLE_NETWORKS[currentNetwork].password);
                                 
                        currentNetwork = (currentNetwork + 1) % NETWORK_COUNT;
                    }
                    
                    connectionAttempts++;
                    if (connectionAttempts >= 10) 
                    {
                        connectionAttempts = 0;
                        totalAttempts++;
                    }
                } 
                else 
                {
                    isWiFiConnected = true;
                    connectionAttempts = 0;
                    totalAttempts = 0;
                    Utilities::LOG_PROD("Network connection established");
                }
            }
        }
        // Rest of the update function...
    }

    /**
     * @brief Process WiFi operations and state updates
     * Handles connection maintenance, status checks, and feedback
     */
    void WiFiManager::checkConnection() 
    {
        Utilities::LOG_SCOPE("WiFiManager::checkConnection()");
        static unsigned long lastTimeCheck = 0;
        static int connectionAttempts = 0;
        static size_t currentNetwork = 0;
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
                        
                        Utilities::LOG_DEBUG("Attempting network: %s", 
                            AVAILABLE_NETWORKS[currentNetwork].ssid);
                        WiFi.begin(AVAILABLE_NETWORKS[currentNetwork].ssid, 
                                 AVAILABLE_NETWORKS[currentNetwork].password);
                        
                        currentNetwork = (currentNetwork + 1) % NETWORK_COUNT;
                    }
                    
                    connectionAttempts++;
                    if (connectionAttempts >= 10) 
                    {
                        connectionAttempts = 0;
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
        Utilities::LOG_SCOPE("WiFiManager::syncTime()");
        static unsigned long lastTimeCheck = 0;
        static int timeAttempts = 0;
        
        try 
        {
            if (!timeInitialized && millis() - lastTimeCheck >= TIME_CHECK_INTERVAL) 
            {
                if (!isWiFiConnected) return false;

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
                        Utilities::LOG_ERROR("Time sync failed after 40 attempts");
                        return false;
                    }
                }
                lastTimeCheck = millis();
            }
        }
        catch (const std::exception& e)
        {
            Utilities::LOG_ERROR("Time sync error: %s", e.what());
            return false;
        }
        catch (...)
        {
            Utilities::LOG_ERROR("Unknown error in time sync");
            return false;
        }
        return false;
    }

    /**
     * @brief Connect to specified WiFi network
     * @param ssid Network identifier
     * @param password Network security key
     * @return True if connection initiated successfully
     */
    bool WiFiManager::connectToWiFi() 
    {
        Utilities::LOG_SCOPE("WiFiManager::connectToWiFi()");
        static size_t currentNetwork = 0;
        static uint8_t totalAttempts = 0;
        
        Utilities::LOG_DEBUG("Starting WiFi connection process");
        WiFi.disconnect(true);
        delay(100);
        
        Utilities::LOG_DEBUG("Attempting network: %s (Attempt %d/%d)", 
            AVAILABLE_NETWORKS[currentNetwork].ssid,
            totalAttempts + 1,
            ROVER_WIFI_MAX_ATTEMPTS);
            
        WiFi.begin(AVAILABLE_NETWORKS[currentNetwork].ssid, 
                   AVAILABLE_NETWORKS[currentNetwork].password);
                   
        currentNetwork = (currentNetwork + 1) % NETWORK_COUNT;
        lastWiFiAttempt = millis();
        isWiFiConnected = false;
        
        return true; // Connection attempt initiated
    }

    // Generic error handler
    void handleError(const char* errorMessage) {
        PC::Utilities::LOG_PROD(errorMessage);
        VisualCortex::RoverManager::setTemporaryExpression(PC::Expression::LOOKING_DOWN, 1000);
    }

    // Example usage in various scenarios
    void processAPIRequest(bool success = false) {  // Parameter with default value
        VisualCortex::RoverManager::setTemporaryExpression(PC::Expression::LOOKING_UP);  // Looking up while thinking
        
        if (success) {
            VisualCortex::RoverManager::setTemporaryExpression(PC::Expression::BIG_SMILE, 1000);
        } else {
            handleError("API request failed");
        }
    }
}