#include "RoverBehaviorManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../PrefrontalCortex/SDManager.h"
#include "../SomatosensoryCortex/AppManager.h"
#include <SPIFFS.h>

// Initialize static members
RoverBehaviorManager::BehaviorState RoverBehaviorManager::currentState = BehaviorState::LOADING;
RoverBehaviorManager::LoadingPhase RoverBehaviorManager::loadingPhase = LoadingPhase::BOOTING;
const char* RoverBehaviorManager::currentStatusMessage = "";
RoverBehaviorManager::BehaviorState RoverBehaviorManager::previousState = BehaviorState::LOADING;
unsigned long RoverBehaviorManager::warningStartTime = 0;
int RoverBehaviorManager::currentBootStep = 0;
bool RoverBehaviorManager::isCountingDown = false;

void RoverBehaviorManager::init() {
    // Start in LOADING state, BOOTING phase
    setState(BehaviorState::LOADING);
    setLoadingPhase(LoadingPhase::BOOTING);
    LEDManager::startLoadingAnimation(); // Optional loading animation
}

void RoverBehaviorManager::update() {
    switch (currentState) {
        case BehaviorState::LOADING:
            handleLoading();
            break;
        case BehaviorState::HOME:
            handleHome();
            break;
        case BehaviorState::MENU:
            handleMenu();
            break;
        case BehaviorState::APP:
            handleApp();
            break;
        case BehaviorState::ERROR:
            handleError();
            break;
        case BehaviorState::WARNING:
            handleWarning();
            break;
        case BehaviorState::FATAL_ERROR:
            handleFatalError();
            break;
    }

    // Decide what to draw
    switch (currentState) {
        case BehaviorState::LOADING:
            RoverViewManager::drawLoadingScreen(currentStatusMessage);
            break;
        case BehaviorState::HOME:
            if (MenuManager::isVisible()) {
                MenuManager::drawMenu();
            } else if (RoverViewManager::hasActiveNotification()) {
                RoverViewManager::drawNotification();
            } else {
                RoverViewManager::drawCurrentView();
            }
            break;
        case BehaviorState::MENU:
            MenuManager::drawMenu();
            break;
        case BehaviorState::APP:
            if (MenuManager::isVisible()) {
                MenuManager::drawMenu();
            } else if (RoverViewManager::hasActiveNotification()) {
                RoverViewManager::drawNotification();
            } else {
                RoverViewManager::drawCurrentView();
            }
            break;
        case BehaviorState::ERROR:
            RoverViewManager::drawLoadingScreen(currentStatusMessage);
            break;
        case BehaviorState::FATAL_ERROR:
            handleFatalError();
            break;
    }

    // Push rendered sprite to display
    spr.pushSprite(0, 0);

    // Add to existing update function
    if (currentState == BehaviorState::WARNING) {
        updateWarningCountdown();
    }
}

RoverBehaviorManager::BehaviorState RoverBehaviorManager::getCurrentState() {
    return currentState;
}

void RoverBehaviorManager::setState(BehaviorState state) {
    currentState = state;
    switch (state) {
        case BehaviorState::LOADING:
            currentStatusMessage = "Loading...";
            LEDManager::startLoadingAnimation();
            break;
        case BehaviorState::HOME:
            currentStatusMessage = "Welcome Home!";
            LEDManager::stopLoadingAnimation();
            break;
        case BehaviorState::MENU:
            currentStatusMessage = "Menu";
            break;
        case BehaviorState::APP:
            currentStatusMessage = "App Running";
            break;
        case BehaviorState::ERROR:
            currentStatusMessage = "Error Occurred!";
            LEDManager::stopLoadingAnimation();
            break;
        case BehaviorState::WARNING:
            currentStatusMessage = "Warning!";
            break;
        case BehaviorState::FATAL_ERROR:
            currentStatusMessage = "Fatal Error!";
            LEDManager::stopLoadingAnimation();
            break;
    }
}

RoverBehaviorManager::LoadingPhase RoverBehaviorManager::getLoadingPhase() {
    return loadingPhase;
}

void RoverBehaviorManager::setLoadingPhase(LoadingPhase phase) {
    loadingPhase = phase;
    switch (phase) {
        case LoadingPhase::BOOTING:
            currentStatusMessage = "Booting Up...";
            break;
        case LoadingPhase::CONNECTING_WIFI:
            currentStatusMessage = "Connecting to Wi-Fi...";
            break;
        case LoadingPhase::SYNCING_TIME:
            currentStatusMessage = "Synchronizing Time...";
            break;
    }
}

const char* RoverBehaviorManager::getStatusMessage() {
    return currentStatusMessage;
}

//----- Main State Handlers -----

void RoverBehaviorManager::handleLoading() {
    switch (loadingPhase) {
        case LoadingPhase::BOOTING:
            handleBooting();
            break;
        case LoadingPhase::CONNECTING_WIFI:
            handleWiFiConnection();
            break;
        case LoadingPhase::SYNCING_TIME:
            handleTimeSync();
            break;
    }
}

void RoverBehaviorManager::handleHome() {
    // Basic updates in home state
    LEDManager::updateLoadingAnimation();
    RoverManager::updateHoverAnimation();
}

void RoverBehaviorManager::handleMenu() {
    Serial.println("handleMenu");
    // The menu system manages its own logic
}

void RoverBehaviorManager::handleApp() {
    if (!AppManager::isAppActive()) {
        setState(BehaviorState::HOME);
    }
}

void RoverBehaviorManager::handleError() {
    // Simple error handling: blink or wait, then reboot
    static unsigned long errorStartTime = 0;
    static bool inError = false;
    if (!inError) {
        inError = true;
        errorStartTime = millis();
    }
    if (millis() - errorStartTime > 5000) {
        ESP.restart();
    }
}

void RoverBehaviorManager::handleFatalError() {
    RoverViewManager::drawErrorScreen(
        RoverViewManager::errorCode,
        RoverViewManager::errorMessage,
        true  // isFatal = true
    );
    
    // Check for rotary button press
    if (UIManager::isRotaryPressed()) {
        ESP.restart();
    }
}

void RoverBehaviorManager::handleWarning() {
    if (!isCountingDown) return;
    
    unsigned long elapsed = millis() - warningStartTime;
    if (elapsed >= RoverBehaviorManager::WARNING_DURATION || UIManager::isRotaryPressed()) {
        // Clear warning state
        isCountingDown = false;
        setState(BehaviorState::IDLE);
        RoverViewManager::isError = false;
        LEDManager::clearErrorPattern();
        return;
    }
    
    updateWarningCountdown();
}

//----- Sub-phase Handlers for LOADING -----
void RoverBehaviorManager::handleBooting() {
    static unsigned long lastMsgChange = 0;
    static int step = 0;
    const unsigned long stepDelay = 800;
    
    Serial.print("Boot step: ");
    Serial.println(step);
    
    if (millis() - lastMsgChange > stepDelay) {
        switch(step) {
            case 0:
                Serial.println("Initializing hardware...");
                currentStatusMessage = "Initializing hardware...";
                break;
            case 1:
                Serial.println("Starting systems...");
                currentStatusMessage = "Starting systems...";
                break;
            case 2:
                Serial.println("Preparing network...");
                currentStatusMessage = "Preparing network...";
                break;
            case 3:
                Serial.println("Almost ready...");
                currentStatusMessage = "Almost ready...";
                break;
        }
        
        lastMsgChange = millis();
        step = (step + 1) % 4;
        
        // After completing one full cycle, move to WiFi phase
        if (step == 0) {
            Serial.println("Moving to WiFi connection phase");
            setLoadingPhase(LoadingPhase::CONNECTING_WIFI);
            WiFiManager::init();
        }
    }
}


void RoverBehaviorManager::handleWiFiConnection() {
    static unsigned long lastAttempt = 0;
    const unsigned long retryDelay = 5000;
    static int retryCount = 0;
    const int MAX_RETRIES = 3;

    if (!WiFiManager::isConnected()) {
        if (millis() - lastAttempt > retryDelay) {
            if (retryCount >= MAX_RETRIES) {
                triggerFatalError(
                    static_cast<uint32_t>(RoverBehaviorManager::StartupErrorCode::WIFI_INIT_FAILED),
                    "Failed to connect to WiFi"
                );
                return;
            }
            WiFiManager::connectToWiFi();
            lastAttempt = millis();
            retryCount++;
        }
    } else {
        setLoadingPhase(LoadingPhase::SYNCING_TIME);
        retryCount = 0;
    }
}

void RoverBehaviorManager::handleTimeSync() {
    static int retryCount = 0;
    const int MAX_RETRIES = 3;
    
    if (!WiFiManager::getTimeInitialized()) {
        if (retryCount >= MAX_RETRIES) {
            triggerError(
                static_cast<uint32_t>(StartupErrorCode::TIME_SYNC_FAILED),
                "Failed to sync time",
                ErrorType::WARNING
            );
            return;
        }
        retryCount++;
    } else {
        setState(BehaviorState::HOME);
    }
}

void RoverBehaviorManager::triggerFatalError(uint32_t errorCode, const char* errorMessage) {
    setState(BehaviorState::FATAL_ERROR);
    RoverViewManager::errorCode = errorCode;
    RoverViewManager::errorMessage = errorMessage;
    RoverViewManager::isError = true;
    RoverViewManager::isFatalError = true;
    
    // Draw error screen
    RoverViewManager::drawErrorScreen(errorCode, errorMessage, true);
}

void RoverBehaviorManager::triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type) {
    // Always log to serial
    Serial.printf("ERROR 0x%08X: %s (Type: %s)\n", 
        errorCode, 
        errorMessage, 
        type == ErrorType::FATAL ? "FATAL" : 
        type == ErrorType::WARNING ? "WARNING" : "SILENT"
    );
    
    // For silent errors, only log to serial and return
    if (type == ErrorType::SILENT) {
        return;
    }
    
    // Handle visible errors
    if (type == ErrorType::FATAL) {
        setState(BehaviorState::FATAL_ERROR);
    } else {
        setState(BehaviorState::WARNING);
        warningStartTime = millis();
        isCountingDown = true;
    }
    
    RoverViewManager::errorCode = errorCode;
    RoverViewManager::errorMessage = errorMessage;
    RoverViewManager::isError = true;
    RoverViewManager::isFatalError = (type == ErrorType::FATAL);
    
    // Play error sound and set LED pattern
    SoundFxManager::playErrorCode(errorCode, type == ErrorType::FATAL);
    LEDManager::setErrorPattern(errorCode, type == ErrorType::FATAL);
    
    // Draw error screen with countdown for warnings
    RoverViewManager::drawErrorScreen(errorCode, errorMessage, type == ErrorType::FATAL);
}

void RoverBehaviorManager::updateWarningCountdown() {
    if (!isCountingDown) return;
    
    unsigned long elapsed = millis() - warningStartTime;
    int remainingSeconds = 3 - (elapsed / 1000);
    
    if (remainingSeconds <= 0) {
        // Warning timeout - clear warning state
        isCountingDown = false;
        setState(BehaviorState::IDLE);
        RoverViewManager::isError = false;
        LEDManager::clearErrorPattern();
        return;
    }
    
    // Update countdown display
    char countdownMsg[32];
    sprintf(countdownMsg, "%s\n\nClearing in %d...", 
        RoverViewManager::errorMessage, 
        remainingSeconds
    );
    RoverViewManager::drawErrorScreen(
        RoverViewManager::errorCode,
        countdownMsg,
        false
    );
}

int RoverBehaviorManager::getCurrentBootStep() {
    return currentBootStep;
}
