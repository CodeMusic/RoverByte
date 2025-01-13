#include "WiFiManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../PrefrontalCortex/PowerManager.h"
#include <FastLED.h>
#include "../PrefrontalCortex/utilities.h"

#define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
#define TIME_CHECK_INTERVAL 500    // Time sync check interval

bool WiFiManager::isRecording = false;
bool WiFiManager::isWiFiConnected = false;
unsigned long WiFiManager::lastWiFiAttempt = 0;
bool WiFiManager::timeInitialized = false;

void WiFiManager::init() {
    checkConnection();
}

void WiFiManager::checkConnection() {
    static unsigned long lastTimeCheck = 0;
    static int wifiAttempts = 0;
    static bool tryingBackup = false;
    const unsigned long CHECK_INTERVAL = 500;
    
    if (isRecording) return;
    
    // First handle WiFi connection
    if (!isWiFiConnected) {
        if (millis() - lastTimeCheck >= CHECK_INTERVAL) {
            if (WiFi.status() != WL_CONNECTED) {
                WiFi.disconnect();
                if (!tryingBackup) {
                    LOG_DEBUG("Trying primary WiFi...");
                    WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
                } else {
                    LOG_DEBUG("Trying backup WiFi...");
                    WiFi.begin(BACKUP_SSID, BACKUP_PASSWORD);
                }
                wifiAttempts++;
                
                if (wifiAttempts >= 20) {  // After 20 attempts (~10 seconds)
                    wifiAttempts = 0;  // Reset counter
                    tryingBackup = !tryingBackup;  // Switch networks
                }
            } else {
                isWiFiConnected = true;
                wifiAttempts = 0;
                LOG_PROD("WiFi connected!");
            }
            lastTimeCheck = millis();
        }
    } else if (!timeInitialized) {
        syncTime();  // Try to sync time if WiFi is connected
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
    if (isRecording) return;
    
    LOG_DEBUG("Starting WiFi connection process");
    WiFi.begin(PRIMARY_SSID, PRIMARY_PASSWORD);
    lastWiFiAttempt = millis();
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
