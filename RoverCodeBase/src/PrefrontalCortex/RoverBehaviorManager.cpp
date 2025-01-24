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
unsigned long RoverBehaviorManager::fatalErrorStartTime = 0;
bool RoverBehaviorManager::isFatalError = false;

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
            updateWarningCountdown();
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
    LEDManager::updateLoadingAnimation();

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
    // Remove auto-reboot completely
    static bool inError = false;
    if (!inError) {
        inError = true;
    }
    // No reboot logic here
}

void RoverBehaviorManager::handleFatalError() {
    if (!RoverViewManager::isFatalError) return;
    
    // Only check for manual reboot via button press, but don't actually reboot
    if (UIManager::isRotaryPressed()) {
        // ESP.restart(); - Removed
    }
}

void RoverBehaviorManager::updateWarningCountdown() {
    if (!isCountingDown && !isFatalError) return;
    
    unsigned long elapsed;
    if (isFatalError) {
        elapsed = millis() - fatalErrorStartTime;
        // Remove reboot logic completely
        return;
    }
    
    // Rest of the warning countdown logic remains the same
    elapsed = millis() - warningStartTime;
    if (elapsed >= WARNING_DURATION) {
        isCountingDown = false;
        setState(BehaviorState::IDLE);
        RoverViewManager::isError = false;
        LEDManager::clearErrorPattern();
        return;
    }
    
    // Update countdown display
    int remainingSeconds = ((WARNING_DURATION - elapsed) / 1000);
    char countdownMsg[32];
    sprintf(countdownMsg, "%s", RoverViewManager::errorMessage);
    RoverViewManager::drawErrorScreen(
        RoverViewManager::errorCode,
        countdownMsg,
        false
    );
}

//----- Sub-phase Handlers for LOADING -----
static int step = 0;
void RoverBehaviorManager::handleBooting() {
    static unsigned long lastMsgChange = 0;
    
    const unsigned long stepDelay = 800;
    
    if (millis() - lastMsgChange > stepDelay) {
        LOG_DEBUG("Boot step: %d", step);
        
        try {
            switch(step) {
                case 0:
                    LOG_DEBUG("Hardware initialization started.");
                    currentStatusMessage = "Initializing hardware...";
                    LEDManager::init();
                    SoundFxManager::init();
                    break;
                case 1:
                    LOG_DEBUG("System startup in progress.");
                    currentStatusMessage = "Starting systems...";
                    PowerManager::init();
                    LOG_DEBUG("Initializing SD card...");
                    SDManager::init();
                    LOG_DEBUG("SD card initialization complete.");
                    break;
                    
                case 2:
                    LOG_DEBUG("Network preparation started.");
                    currentStatusMessage = "Preparing network...";
                    WiFiManager::init();
                    break;
                    
                case 3:
                    LOG_DEBUG("Boot process nearing completion.");
                    currentStatusMessage = "Almost ready...";
                    RoverViewManager::init();
                    UIManager::init();
                    MenuManager::init();
                    break;
            }

            
            currentBootStep = step;
            lastMsgChange = millis();
            step++;
            
            if (step >= 4) {
                LOG_DEBUG("Transitioning to WiFi connection phase.");
                setLoadingPhase(LoadingPhase::CONNECTING_WIFI);
            }
            RoverViewManager::drawLoadingScreen(currentStatusMessage);
            
        } catch (const std::exception& e) {
            switch(step) {
                case 0:
                    triggerFatalError(
                        static_cast<uint32_t>(StartupErrorCode::STORAGE_INIT_FAILED),
                        "Storage initialization failed"
                    );
                    break;
                case 1:
                    triggerFatalError(
                        static_cast<uint32_t>(StartupErrorCode::POWER_INIT_FAILED),
                        "Power system initialization failed"
                    );
                    break;
                case 2:
                    triggerFatalError(
                        static_cast<uint32_t>(StartupErrorCode::UI_INIT_FAILED),
                        "UI system initialization failed"
                    );
                    break;
                case 3:
                    triggerError(
                        static_cast<uint32_t>(StartupErrorCode::WIFI_INIT_FAILED),
                        "WiFi init failed",
                        ErrorType::FATAL
                    );
                    break;
            }
            return;
        }
    }
}


void RoverBehaviorManager::handleWiFiConnection() {
    static unsigned long startAttempt = millis();
    static bool timeoutWarningShown = false;
    const unsigned long WIFI_TIMEOUT = 10000; // 10 second timeout
    
    if (!timeoutWarningShown && millis() - startAttempt > WIFI_TIMEOUT) {
        // Only show timeout warning once
        timeoutWarningShown = true;
        triggerError(
            static_cast<uint32_t>(StartupErrorCode::WIFI_INIT_FAILED),
            "WiFi connection timeout",
            ErrorType::WARNING
        );
        setLoadingPhase(LoadingPhase::SYNCING_TIME); // Skip to next phase
        return;
    }
    
    // Continue with normal WiFi connection attempts
    if (WiFiManager::isConnected()) {
        setLoadingPhase(LoadingPhase::SYNCING_TIME);
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
            setState(BehaviorState::HOME); // Continue to home even if time sync fails
            return;
        }
        WiFiManager::syncTime(); // Actually try to sync
        retryCount++;
    } else {
        setState(BehaviorState::HOME);
        retryCount = 0; // Reset for next time
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

int RoverBehaviorManager::getCurrentBootStep() {
    return currentBootStep;
}
