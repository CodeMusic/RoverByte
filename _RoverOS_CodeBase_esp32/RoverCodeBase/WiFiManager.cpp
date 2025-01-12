#include "WiFiManager.h"
#include "LEDManager.h"
#include "PowerManager.h"
#include <FastLED.h>
#include "utilities.h"

#define WIFI_RETRY_INTERVAL 60000  // 1 minute between retry attempts
extern CRGB leds[];  // Declare the external leds array

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
        (millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL || lastWiFiAttempt == 0)) {
        
        // Try primary network first
        LOG_DEBUG("Attempting primary WiFi connection...");
        WiFi.begin(primarySSID, primaryPassword);
        
        // Wait for primary WiFi with short timeout
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startAttemptTime < 10000) {  // 10 second timeout
            delay(500);
            leds[0] = CRGB::Blue;
            FastLED.show();
            delay(250);
            leds[0] = CRGB::Black;
            FastLED.show();
            delay(250);
        }
        
        // If primary fails, try backup network
        if (WiFi.status() != WL_CONNECTED) {
            LOG_DEBUG("Primary WiFi failed, trying backup network...");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(backupSSID, backupPassword);
            
            // Wait for backup WiFi
            startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && 
                   millis() - startAttemptTime < 10000) {  // 10 second timeout
                delay(500);
                leds[0] = CRGB::Purple;  // Different color for backup network
                FastLED.show();
                delay(250);
                leds[0] = CRGB::Black;
                FastLED.show();
                delay(250);
            }
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
    // Try primary network
    WiFi.begin(primarySSID, primaryPassword);
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
        delay(500);
        LOG_DEBUG(".");
        LEDManager::updateLoadingAnimation();
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        LOG_PROD("Primary WiFi failed, trying backup...");
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(backupSSID, backupPassword);
        
        startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
            delay(500);
            LOG_DEBUG(".");
            LEDManager::updateLoadingAnimation();
        }
    }
    
    isWiFiConnected = (WiFi.status() == WL_CONNECTED);
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
