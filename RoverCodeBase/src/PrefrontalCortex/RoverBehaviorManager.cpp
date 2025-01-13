#include "RoverBehaviorManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/SDManager.h"
#include "../PsychicCortex/NFCManager.h"
#include <SPIFFS.h>

RoverBehaviorManager::BehaviorState RoverBehaviorManager::currentState = RoverBehaviorManager::BOOTING;
const char* RoverBehaviorManager::currentStatusMessage = "";

void RoverBehaviorManager::init() {
    setState(BOOTING);
}

void RoverBehaviorManager::update() {
    // Update state first
    switch(currentState) {
        case BOOTING:
            handleBooting();
            break;
        case CONNECTING_WIFI:
            handleWiFiConnection();
            break;
        case SYNCING_TIME:
            handleTimeSync();
            break;
        case ERROR:
            handleError();
            break;
        case READY:
            handleReady();
            break;
    }
    
    // Then handle drawing based on state
    if (currentState != READY) {
        RoverViewManager::drawLoadingScreen(currentStatusMessage);
    } else {
        if (RoverViewManager::hasActiveNotification()) {
            RoverViewManager::drawNotification();
        } else {
            RoverViewManager::drawCurrentView();
        }
    }
    spr.pushSprite(0, 0);
}

void RoverBehaviorManager::handleBooting() {
    static const char* bootMessages[] = {
        "Initializing systems...",
        "Starting up...",
        "Loading core modules...",
        "Ready to connect!"
    };
    static int messageIndex = 0;
    static unsigned long lastMessageChange = 0;
    static int cycleCount = 0;
    
    if (millis() - lastMessageChange > 800) {
        currentStatusMessage = bootMessages[messageIndex];
        messageIndex = (messageIndex + 1) % 4;
        lastMessageChange = millis();
        
        if (messageIndex == 0) {
            cycleCount++;
            if (cycleCount >= 2) {  // Complete two full cycles before WiFi
                setState(CONNECTING_WIFI);
                WiFiManager::init();  // Ensure fresh WiFi state
                delay(100);
            }
        }
    }
}

void RoverBehaviorManager::handleWiFiConnection() {
    static unsigned long lastAttempt = 0;
    static const char* wifiMessages[] = {
        "Scanning networks...",
        "Connecting to WiFi...",
        "Establishing connection...",
        "Verifying connection..."
    };
    static int messageIndex = 0;
    static unsigned long lastMessageChange = 0;
    static bool connectionStarted = false;
    static unsigned long lastLogTime = 0;
    const unsigned long LOG_INTERVAL = 5000;  // Only log every 5 seconds
    
    // Throttle debug logging
    if (millis() - lastLogTime >= LOG_INTERVAL) {
        Serial.println("Attempting WiFi connection...");
        lastLogTime = millis();
    }
    
    // Initial connection attempt
    if (!connectionStarted) {
        Serial.println("Starting initial WiFi connection");
        WiFiManager::init();
        delay(100);
        WiFiManager::connectToWiFi();
        connectionStarted = true;
        lastAttempt = millis();
    }
    
    // Check connection status
    WiFiManager::checkConnection();
    
    if (!WiFiManager::isConnected()) {
        // Only retry connection if enough time has passed
        if (millis() - lastAttempt > 5000) {
            WiFiManager::connectToWiFi();
            lastAttempt = millis();
        }
        
        // Update status message
        if (millis() - lastMessageChange > 800) {
            currentStatusMessage = wifiMessages[messageIndex];
            messageIndex = (messageIndex + 1) % 4;
            lastMessageChange = millis();
        }
    } else {
        Serial.println("WiFi connected, moving to time sync");
        setState(SYNCING_TIME);
        currentStatusMessage = "WiFi Connected!";
    }
}

void RoverBehaviorManager::handleTimeSync() {
    if (!WiFiManager::getTimeInitialized()) {
        Serial.println("Time not initialized, synchronizing");
        static const char* syncMessages[] = {
            "Synchronizing time...",
            "Fetching NTP data...",
            "Updating clock...",
            "Finalizing time sync..."
        };
        static int messageIndex = 0;
        static unsigned long lastMessageChange = 0;
        
        if (millis() - lastMessageChange > 1000) {
            currentStatusMessage = syncMessages[messageIndex];
            messageIndex = (messageIndex + 1) % 4;
            lastMessageChange = millis();
        }
        
        WiFiManager::syncTime();
    } else {
        setState(READY);
        currentStatusMessage = "Ready! *tail wag*";
        LEDManager::setMode(Mode::FULL_MODE);
        LEDManager::stopLoadingAnimation();
        SoundFxManager::playStartupSound();
    }
}

void RoverBehaviorManager::handleReady() {
    LEDManager::updateLoadingAnimation();
    RoverManager::updateHoverAnimation();
}

void RoverBehaviorManager::handleError() {
    static unsigned long lastErrorBlink = 0;
    static bool errorLedState = false;
    static unsigned long errorStartTime = millis();
    
    // After 5 seconds of error, try rebooting
    if (millis() - errorStartTime > 5000) {
        ESP.restart();
    }
    
    if (millis() - lastErrorBlink > 500) {
        errorLedState = !errorLedState;
        if (errorLedState) {
            LEDManager::setLED(0, CRGB::Red);
        } else {
            LEDManager::setLED(0, CRGB::Black);
        }
        LEDManager::showLEDs();
        lastErrorBlink = millis();
    }
}

void RoverBehaviorManager::setState(BehaviorState state) {
    currentState = state;
    switch(state) {
        case ERROR:
            currentStatusMessage = "Error occurred!";
            break;
        case READY:
            currentStatusMessage = "Ready!";
            LEDManager::flashSuccess();
            break;
        default:
            break;
    }
}

void RoverBehaviorManager::handleSideButton() {
    static bool earsUp = false;
    static unsigned long scanStartTime = 0;
    const unsigned long SCAN_TIMEOUT = 5000; // 5 second timeout
    
    if (!earsUp) {
        // Start NFC scan and raise ears
        RoverManager::setEarsUp();
        earsUp = true;
        scanStartTime = millis();
        NFCManager::handleSideButtonPress();  // This starts the NFC flow
    } else {
        // Only check timeout, not card presence yet
        if (millis() - scanStartTime >= SCAN_TIMEOUT) {
            RoverManager::setEarsDown();
            earsUp = false;
            if (NFCManager::isCardPresent()) {
                // Card was found during scan
                RoverManager::setTemporaryExpression(RoverManager::EXCITED, 2000);
            } else {
                // No card found after timeout
                RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 1000);
                RoverViewManager::showNotification("No Card", "Please try again", "NFC", 2000);
            }
        }
    }
} 