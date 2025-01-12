#include "WiFiManager.h"
#include "../VisualCortex/LEDManager.h"
#include "PowerManager.h"
#include <FastLED.h>
#include "utilities.h"

#define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts

bool WiFiManager::isRecording = false;
bool WiFiManager::isWiFiConnected = false;
unsigned long WiFiManager::lastWiFiAttempt = 0;
const char* WiFiManager::primarySSID = nullptr;
const char* WiFiManager::primaryPassword = nullptr;
const char* WiFiManager::backupSSID = nullptr;
const char* WiFiManager::backupPassword = nullptr;
bool WiFiManager::timeInitialized = false;


void WiFiManager::setCredentials(const char* primary_ssid, const char* primary_pass,
                               const char* backup_ssid, const char* backup_pass) {
    primarySSID = primary_ssid;
    primaryPassword = primary_pass;
    backupSSID = backup_ssid;
    backupPassword = backup_pass;
}

void WiFiManager::init() {
    checkConnection();
}

void WiFiManager::checkConnection() {
    if (isRecording) return;  // Skip WiFi connection attempts while recording
    
    if (!isWiFiConnected && 
        (millis() - lastWiFiAttempt > WIFI_RETRY_INTERVAL)) {
        
        // Try primary network first
        if (primarySSID && primaryPassword) {
            LOG_DEBUG("Attempting to connect to primary network: %s", primarySSID);
            WiFi.begin(primarySSID, primaryPassword);
            
            // Visual indicator for primary network attempt
            LEDManager::setLED(0, CRGB::Blue);
            LEDManager::showLEDs();
            delay(250);
            LEDManager::setLED(0, CRGB::Black);
            LEDManager::showLEDs();
            delay(250);
            
            // Wait for connection
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                delay(500);
                attempts++;
            }
        }
        
        // If primary failed and backup credentials exist, try backup
        if (WiFi.status() != WL_CONNECTED && backupSSID && backupPassword) {
            LOG_DEBUG("Primary connection failed, trying backup network: %s", backupSSID);
            WiFi.begin(backupSSID, backupPassword);
            
            // Visual indicator for backup network attempt
            LEDManager::setLED(0, CRGB::Purple);
            LEDManager::showLEDs();
            delay(250);
            LEDManager::setLED(0, CRGB::Black);
            LEDManager::showLEDs();
            delay(250);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            LOG_PROD("WiFi connected to %s", WiFi.SSID().c_str());
            LOG_PROD("IP address: %s", WiFi.localIP().toString().c_str());
            
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
            
            int attempts = 0;
            while (time(nullptr) < 1000000000 && attempts < 10) {
                delay(500);
                attempts++;
            }
            
            if (time(nullptr) > 1000000000) {
                LOG_PROD("Time synchronized successfully");
                isWiFiConnected = true;
                timeInitialized = true;
                LEDManager::stopLoadingAnimation();
            }
        } else {
            LOG_DEBUG("All WiFi connections failed");
            WiFi.disconnect();
        }
        
        lastWiFiAttempt = millis();
    }
}

void WiFiManager::connectToWiFi() {
    if (isRecording) return;  // Skip WiFi connection attempts while recording
    
    // Try primary network first
    LOG_DEBUG("Attempting primary WiFi connection...");
    WiFi.begin(primarySSID, primaryPassword);
    
    // Wait for primary WiFi with short timeout
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < 10000) {  // 10 second timeout
        delay(500);
        LOG_DEBUG(".");
        LEDManager::updateLoadingAnimation();
    }
    
    // If primary failed, try backup network
    if (WiFi.status() != WL_CONNECTED) {
        LOG_DEBUG("\nPrimary connection failed, trying backup...");
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(backupSSID, backupPassword);
        
        startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startAttemptTime < 20000) {  // 20 second timeout
            delay(500);
            LOG_DEBUG(".");
            LEDManager::updateLoadingAnimation();
        }
    }
    
    // Update connection status
    isWiFiConnected = (WiFi.status() == WL_CONNECTED);
    
    if (isWiFiConnected) {
        LOG_PROD("WiFi connected to %s", WiFi.SSID().c_str());
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
        timeInitialized = true;
    } else {
        LOG_PROD("WiFi connection failed");
    }
    
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

bool WiFiManager::getTimeInitialized() {
    return timeInitialized;
}
