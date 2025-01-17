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
RoverBehaviorManager::BehaviorState RoverBehaviorManager::currentState = RoverBehaviorManager::LOADING;
RoverBehaviorManager::LoadingPhase RoverBehaviorManager::loadingPhase = RoverBehaviorManager::BOOTING;
const char* RoverBehaviorManager::currentStatusMessage = "";

void RoverBehaviorManager::init() {
    // Start in LOADING state, BOOTING phase
    setState(LOADING);
    setLoadingPhase(BOOTING);
    LEDManager::startLoadingAnimation(); // Optional loading animation
}

void RoverBehaviorManager::update() {
    switch (currentState) {
        case LOADING:
            handleLoading();
            break;
        case HOME:
            handleHome();
            break;
        case MENU:
            handleMenu();
            break;
        case APP:
            handleApp();
            break;
        case ERROR:
            handleError();
            break;
    }

    // Decide what to draw
    switch (currentState) {
        case LOADING:
            RoverViewManager::drawLoadingScreen(currentStatusMessage);
            break;
        case HOME:
            if (MenuManager::isVisible()) {
                MenuManager::drawMenu();
            } else if (RoverViewManager::hasActiveNotification()) {
                RoverViewManager::drawNotification();
            } else {
                RoverViewManager::drawCurrentView();
            }
            break;
        case MENU:
            MenuManager::drawMenu();
            break;
        case APP:
            // If in an app (e.g., a game), could manage sub-logic here
            if (MenuManager::isVisible()) {
                MenuManager::drawMenu();
            } else if (RoverViewManager::hasActiveNotification()) {
                RoverViewManager::drawNotification();
            } else {
                RoverViewManager::drawCurrentView();
            }
            break;
        case ERROR:
            RoverViewManager::drawLoadingScreen(currentStatusMessage);
            break;
    }

    // Push rendered sprite to display
    spr.pushSprite(0, 0);
}

RoverBehaviorManager::BehaviorState RoverBehaviorManager::getCurrentState() {
    return currentState;
}

void RoverBehaviorManager::setState(BehaviorState state) {
    currentState = state;
    switch (state) {
        case LOADING:
            currentStatusMessage = "Loading...";
            LEDManager::startLoadingAnimation();
            break;
        case HOME:
            currentStatusMessage = "Welcome Home!";
            LEDManager::stopLoadingAnimation();
            break;
        case MENU:
            currentStatusMessage = "Menu";
            break;
        case APP:
            currentStatusMessage = "App Running";
            break;
        case ERROR:
            currentStatusMessage = "Error Occurred!";
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
        case BOOTING:
            currentStatusMessage = "Booting Up...";
            break;
        case CONNECTING_WIFI:
            currentStatusMessage = "Connecting to Wi-Fi...";
            break;
        case SYNCING_TIME:
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
        case BOOTING:
            handleBooting();
            break;
        case CONNECTING_WIFI:
            handleWiFiConnection();
            break;
        case SYNCING_TIME:
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
    // The menu system manages its own logic
}

void RoverBehaviorManager::handleApp() {
    if (!AppManager::isAppActive()) {
        setState(HOME);
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

//----- Sub-phase Handlers for LOADING -----

void RoverBehaviorManager::handleBooting() {
    static unsigned long lastMsgChange = 0;
    static int step = 0;
    const unsigned long stepDelay = 800;
    const char* bootMsgs[] = {
        "Initializing modules...",
        "Starting up...",
        "Preparing environment...",
        "Entering next phase..."
    };

    if (millis() - lastMsgChange > stepDelay) {
        currentStatusMessage = bootMsgs[step];
        lastMsgChange = millis();
        step = (step + 1) % 4;

        // After two cycles, move to WiFi
        static int cycles = 0;
        if (step == 0) {
            cycles++;
            if (cycles >= 2) {
                setLoadingPhase(CONNECTING_WIFI);
                WiFiManager::init();
                delay(100);
            }
        }
    }
}

void RoverBehaviorManager::handleWiFiConnection() {
    static unsigned long lastAttempt = 0;
    const unsigned long retryDelay = 5000;

    if (!WiFiManager::isConnected()) {
        if (millis() - lastAttempt > retryDelay) {
            WiFiManager::connectToWiFi();
            lastAttempt = millis();
        }
    } else {
        setLoadingPhase(SYNCING_TIME);
    }
}

void RoverBehaviorManager::handleTimeSync() {
    if (WiFiManager::getTimeInitialized()) {
        setState(HOME);
    } else {
        WiFiManager::syncTime();
    }
} 