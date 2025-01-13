#include "WiFiManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"

#define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
#define TIME_CHECK_INTERVAL 500    // Time sync check interval


bool WiFiManager::isWiFiConnected = false;
unsigned long WiFiManager::lastWiFiAttempt = 0;
bool WiFiManager::timeInitialized = false;

void WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);  // Disconnect and clear credentials
    delay(100);
}

void WiFiManager::checkConnection() {
    static unsigned long lastTimeCheck = 0;
    static int wifiAttempts = 0;
    static bool tryingBackup = false;
    const unsigned long CHECK_INTERVAL = 500;
    

    if (!isWiFiConnected) {
        if (millis() - lastTimeCheck >= CHECK_INTERVAL) {
            lastTimeCheck = millis();
            
            if (WiFi.status() != WL_CONNECTED) {
                LOG_DEBUG("WiFi Status: %d", WiFi.status());
                
                if (wifiAttempts == 0) {
                    WiFi.disconnect(true);
                    delay(100);
                    if (!tryingBackup) {
                        LOG_DEBUG("Trying primary WiFi...");
                        WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
                    } else {
                        LOG_DEBUG("Trying backup WiFi...");
                        WiFi.begin(BACKUP_SSID, BACKUP_PASSWORD);
                    }
                }
                
                wifiAttempts++;
                if (wifiAttempts >= 10) {  // Try each network for 5 seconds
                    wifiAttempts = 0;
                    tryingBackup = !tryingBackup;
                }
            } else {
                isWiFiConnected = true;
                wifiAttempts = 0;
                LOG_PROD("WiFi connected!");
            }
        }
    } else if (!timeInitialized) {
        syncTime();
    }
}

void WiFiManager::syncTime() {
    static unsigned long lastTimeCheck = 0;
    static int timeAttempts = 0;
    
    if (!timeInitialized && millis() - lastTimeCheck >= TIME_CHECK_INTERVAL) {
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
        if (time(nullptr) > 1000000000) {
            timeInitialized = true;
            LOG_PROD("Time sync complete");
        } else {
            timeAttempts++;
            if (timeAttempts >= 40) {
                timeAttempts = 0;
            }
        }
        lastTimeCheck = millis();
    }
}

void WiFiManager::connectToWiFi() {
    Serial.println("Starting WiFi connection process");
    WiFi.disconnect(true);  // Ensure clean connection attempt
    delay(100);
    WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
    lastWiFiAttempt = millis();
    isWiFiConnected = false;  // Reset connection state
    checkConnection();
}

// Generic error handler
void handleError(const char* errorMessage) {
    LOG_PROD("Error: %s", errorMessage);
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 1000);
}

// For radio button release
void handleRadioButtonRelease() {
    // ... existing radio button code ...
    
    delay(1000);  // Wait a second
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP, 1000);
}

// Example usage in various scenarios
void processAPIRequest(bool success = false) {  // Parameter with default value
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP);  // Looking up while thinking
    
    if (success) {
        RoverManager::setTemporaryExpression(RoverManager::BIG_SMILE, 1000);
    } else {
        handleError("API request failed");
    }
}
