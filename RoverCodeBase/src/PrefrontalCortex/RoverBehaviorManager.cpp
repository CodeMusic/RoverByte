#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include "../CorpusCallosum/SynapticPathways.h"
#include "RoverBehaviorManager.h"
#include "Utilities.h"
#include "SPIManager.h"
#include "SDManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../GameCortex/AppManager.h"
#include "../GameCortex/AppRegistration.h"
#include "../SomatosensoryCortex/UIManager.h"
#include <SPIFFS.h>

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using VC::RoverViewManager;
    using VC::LEDManager;
    using VC::RoverManager;
    using SC::MenuManager;
    using SC::UIManager;
    using AC::SoundFxManager;
    using GC::AppManager;
    using GC::AppRegistration;
    using PSY::WiFiManager;  // Use PSY namespace for WiFiManager

    // Static member initialization
    bool RoverBehaviorManager::initialized = false;
    BehaviorState RoverBehaviorManager::currentState = BehaviorState::INITIALIZING;
    String RoverBehaviorManager::statusMessage = "Starting...";

    void RoverBehaviorManager::init() 
    {
        if (initialized) 
        {
            Utilities::LOG_DEBUG("RoverBehaviorManager already initialized");
            return;
        }

        try 
        {
            // Initialize SD card first
            SDManager::init();
            Utilities::LOG_DEBUG("SD Manager initialized");

            // Initialize other managers
            LEDManager::init();
            RoverViewManager::init();
            MenuManager::init();
            SoundFxManager::init();
            AppManager::init();

            initialized = true;
            Utilities::LOG_DEBUG("RoverBehaviorManager initialized successfully");
        }
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("RoverBehaviorManager init failed: %s", e.what());
            throw;
        }
    }

    void RoverBehaviorManager::setState(BehaviorState newState) 
    {
        if (currentState == newState) return;

        currentState = newState;
        
        switch (currentState) 
        {
            case BehaviorState::FULL_DISPLAY:
                LEDManager::setEncodingMode(VC::EncodingModes::FULL_MODE);
                break;

            case BehaviorState::MENU_MODE:
                LEDManager::setEncodingMode(VC::EncodingModes::MENU_MODE);
                break;

            default:
                break;
        }
    }

    bool RoverBehaviorManager::IsInitialized() {
        return initialized;
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
                RoverViewManager::drawLoadingScreen(statusMessage);
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
                RoverViewManager::drawLoadingScreen(statusMessage);
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

    BehaviorState RoverBehaviorManager::getCurrentState() {
        return currentState;
    }

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
        Utilities::LOG_DEBUG("handleMenu...");
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
    void RoverBehaviorManager::handleBooting() {
        static unsigned long lastMsgChange = 0;
        static int step = 0;
        const unsigned long stepDelay = 800;
        
        if (millis() - lastMsgChange > stepDelay) {
            try {
                switch(step) {
                    case 0:
                        statusMessage = "Initializing hardware...";
                        // Core hardware already initialized in init()
                        break;
                        
                    case 1:
                        statusMessage = "Loading display...";
                        RoverViewManager::init();
                        break;
                    case 2:
                        statusMessage = "Initializing Sound...";
                        SoundFxManager::init();
                        break;
                    case 3:
                        statusMessage = "Starting UI...";
                        UIManager::init();
                        break;
                    case 4:
                        statusMessage = "Preparing apps...";
                        MenuManager::init();
                        break;
                    case 5:
                        statusMessage = "Registering apps...";
                        AppManager::init();
                        if (AppManager::isInitialized()) {
                            AppRegistration::registerDefaultApps();
                        }
                        break;
                }
                
                currentBootStep = step;
                lastMsgChange = millis();
                
                // Draw loading screen before incrementing step
                RoverViewManager::drawLoadingScreen(statusMessage);
                
                step++;
                
                if (step >= 5) {
                    Utilities::LOG_DEBUG("Boot sequence complete, moving to WiFi phase");
                    setLoadingPhase(LoadingPhase::CONNECTING_WIFI);
                    step = 0;
                }
                
            } catch (const std::exception& e) {
                Utilities::LOG_ERROR("Boot step %d failed: %s", step, e.what());
                switch(step) {
                    case 1:
                        triggerFatalError(
                            static_cast<uint32_t>(StartupErrorCode::DISPLAY_INIT_FAILED),
                            "Display initialization failed"
                        );
                        break;
                    case 2:
                        triggerFatalError(
                            static_cast<uint32_t>(StartupErrorCode::UI_INIT_FAILED),
                            "UI initialization failed"
                        );
                        break;
                    case 3:
                    case 4:
                        triggerFatalError(
                            static_cast<uint32_t>(StartupErrorCode::APP_INIT_FAILED),
                            "App initialization failed"
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
        const unsigned long WIFI_TIMEOUT = 20000; // 20 second timeout

        if (!WiFiManager::isConnected()) {
            WiFiManager::init();
            delay(250);
            WiFiManager::checkConnection();
        }
        

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

}