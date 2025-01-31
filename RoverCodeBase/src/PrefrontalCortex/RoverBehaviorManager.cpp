#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include "../CorpusCallosum/SynapticPathways.h"
#include "../MotorCortex/PinDefinitions.h"
#include "RoverBehaviorManager.h"
#include "utilities.h"
#include "SPIManager.h"
#include "SDManager.h"
#include "../PsychicCortex/WiFiManager.h"
#include "../SomatosensoryCortex/UIManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../GameCortex/AppManager.h"
#include "../GameCortex/AppRegistration.h"
#include <SPIFFS.h>

namespace PrefrontalCortex 
{
    // Namespace aliases for cleaner cognitive pathways
    namespace PC = PrefrontalCortex;
    using namespace CorpusCallosum;
    using VC::RoverViewManager;
    using VC::LEDManager;
    using VC::RoverManager;
    using SC::MenuManager;
    using AC::SoundFxManager;
    using GC::AppManager;
    using GC::AppRegistration;
    using PSY::WiFiManager;

    // Initialize neural state variables
    RoverTypes::BehaviorState RoverBehaviorManager::currentState = RoverTypes::BehaviorState::LOADING;
    RoverTypes::LoadingPhase RoverBehaviorManager::loadingPhase = RoverTypes::LoadingPhase::BOOTING;
    const char* RoverBehaviorManager::currentStatusMessage = "Initializing neural pathways...";
    RoverTypes::BehaviorState RoverBehaviorManager::previousState = RoverTypes::BehaviorState::LOADING;
    unsigned long RoverBehaviorManager::warningStartTime = 0;
    bool RoverBehaviorManager::isCountingDown = false;
    int RoverBehaviorManager::currentBootStep = 0;
    unsigned long RoverBehaviorManager::fatalErrorStartTime = 0;
    bool RoverBehaviorManager::isFatalError = false;
    VisualTypes::VisualPattern RoverBehaviorManager::pattern = VisualTypes::VisualPattern::NONE;
    String RoverBehaviorManager::statusMessage = "";
    bool RoverBehaviorManager::initialized = false;

    void RoverBehaviorManager::init() 
    {
        try 
        {
            // Initialize core neural pathways
            SDManager::init(BOARD_SD_CS);
            Utilities::LOG_DEBUG("Memory pathways initialized");

            // Initialize sensory processing systems
            LEDManager::init();
            RoverViewManager::init();
            MenuManager::init();
            SoundFxManager::init();
            AppManager::init();

            initialized = true;
            Utilities::LOG_DEBUG("Cognitive systems initialized successfully");
        }
        catch (const std::exception& e) 
        {
            Utilities::LOG_ERROR("Cognitive initialization failed: %s", e.what());
            throw;
        }
    }

    void RoverBehaviorManager::setState(RoverTypes::BehaviorState newState) 
    {
        if (currentState == newState) return;

        currentState = newState;
        
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::FULL_DISPLAY:
                LEDManager::setEncodingMode(VisualTypes::EncodingModes::FULL_MODE);
                break;

            case RoverTypes::BehaviorState::MENU_MODE:
                LEDManager::setEncodingMode(VisualTypes::EncodingModes::MENU_MODE);
                break;

            default:
                // No LED encoding mode change for other states
                break;
        }
    }

    bool RoverBehaviorManager::IsInitialized() 
    {
        return initialized;
    }

    void RoverBehaviorManager::update() 
    {
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::LOADING:
                handleLoading();
                break;
            case RoverTypes::BehaviorState::HOME:
                handleHome();
                break;
            case RoverTypes::BehaviorState::MENU:
                handleMenu();
                break;
            case RoverTypes::BehaviorState::APP:
                handleApp();
                break;
            case RoverTypes::BehaviorState::ERROR:
                handleError();
                break;
            case RoverTypes::BehaviorState::WARNING:
                updateWarningCountdown();
                break;
            case RoverTypes::BehaviorState::FATAL_ERROR:
                handleFatalError();
                break;
        }

        // Decide what to draw
        switch (currentState) 
        {
            case RoverTypes::BehaviorState::LOADING:
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                break;
            case RoverTypes::BehaviorState::HOME:
                if (MenuManager::isVisible()) 
                {
                    MenuManager::drawMenu();
                } 
                else if (RoverViewManager::hasActiveNotification()) 
                {
                    RoverViewManager::drawNotification();
                } 
                else 
                {
                    RoverViewManager::drawCurrentView();
                }
                break;
            case RoverTypes::BehaviorState::MENU:
                MenuManager::drawMenu();
                break;
            case RoverTypes::BehaviorState::APP:
                if (MenuManager::isVisible()) {
                    MenuManager::drawMenu();
                } else if (RoverViewManager::hasActiveNotification()) {
                    RoverViewManager::drawNotification();
                } else {
                    RoverViewManager::drawCurrentView();
                }
                break;
            case RoverTypes::BehaviorState::ERROR:
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                break;
            case RoverTypes::BehaviorState::FATAL_ERROR:
                handleFatalError();
                break;
        }

        // Add to existing update function
        if (currentState == RoverTypes::BehaviorState::WARNING) {
            updateWarningCountdown();
        }
    }

    RoverTypes::BehaviorState RoverBehaviorManager::getCurrentState() {
        return currentState;
    }

    void RoverBehaviorManager::handleLoading() 
    {
        LEDManager::updateLoadingAnimation();

        switch (loadingPhase) 
        {
            case RoverTypes::LoadingPhase::BOOTING:
                handleBooting();
                break;
            case RoverTypes::LoadingPhase::CONNECTING_WIFI:
                handleWiFiConnection();
                break;
            case RoverTypes::LoadingPhase::SYNCING_TIME:
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
            setState(RoverTypes::BehaviorState::HOME);
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
        if (SC::UIManager::isRotaryPressed()) {
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
            setState(RoverTypes::BehaviorState::IDLE);
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
    void RoverBehaviorManager::handleBooting() 
    {
        static unsigned long lastMsgChange = 0;
        static int step = 0;
        const unsigned long stepDelay = 800;
        
        if (millis() - lastMsgChange > stepDelay) 
        {
            try 
            {
                switch(step) 
                {
                    case 0:
                        currentStatusMessage = "Initializing hardware...";
                        break;
                    case 1:
                        currentStatusMessage = "Loading display...";
                        RoverViewManager::init();
                        break;
                    case 2:
                        currentStatusMessage = "Initializing Sound...";
                        SoundFxManager::init();
                        break;
                    case 3:
                        currentStatusMessage = "Starting UI...";
                        SC::UIManager::init();
                        break;
                    case 4:
                        currentStatusMessage = "Preparing apps...";
                        MenuManager::init();
                        break;
                    case 5:
                        currentStatusMessage = "Registering apps...";
                        AppManager::init();
                        if (AppManager::isInitialized()) {
                            AppRegistration::registerDefaultApps();
                        }
                        break;
                }
                
                currentBootStep = step;
                lastMsgChange = millis();
                
                // Draw loading screen before incrementing step
                RoverViewManager::drawLoadingScreen(currentStatusMessage);
                
                step++;
                
                if (step >= 5) 
                {
                    Utilities::LOG_DEBUG("Boot sequence complete, moving to WiFi phase");
                    setLoadingPhase(RoverTypes::LoadingPhase::CONNECTING_WIFI);
                    step = 0;
                }
            } 
            catch (const std::exception& e) 
            {
                Utilities::LOG_ERROR("Boot step %d failed: %s", step, e.what());
                switch(step) 
                {
                    case 1:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::DISPLAY_INIT_FAILED),
                            "Display initialization failed"
                        );
                        break;
                    case 2:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::UI_INIT_FAILED),
                            "UI initialization failed"
                        );
                        break;
                    case 3:
                    case 4:
                        triggerFatalError(
                            static_cast<uint32_t>(RoverTypes::StartupErrorCode::APP_INIT_FAILED),
                            "App initialization failed"
                        );
                        break;
                }
                return;
            }
        }
    }

    void RoverBehaviorManager::handleWiFiConnection() 
    {
        static unsigned long startAttempt = millis();
        static bool timeoutWarningShown = false;
        const unsigned long WIFI_TIMEOUT = 20000;

        if (!PSY::WiFiManager::isConnected()) 
        {
            PSY::WiFiManager::init();
            delay(250);
            PSY::WiFiManager::checkConnection();
        }

        if (!timeoutWarningShown && millis() - startAttempt > WIFI_TIMEOUT) 
        {
            timeoutWarningShown = true;
            triggerError(
                static_cast<uint32_t>(RoverTypes::StartupErrorCode::WIFI_INIT_FAILED),
                "WiFi connection timeout",
                ErrorType::WARNING
            );
            setLoadingPhase(RoverTypes::LoadingPhase::SYNCING_TIME);
            return;
        }
        
        if (PSY::WiFiManager::isConnected()) 
        {
            setLoadingPhase(RoverTypes::LoadingPhase::SYNCING_TIME);
        }
    }

    void RoverBehaviorManager::handleTimeSync() 
    {
        static int retryCount = 0;
        const int MAX_RETRIES = 3;
        
        if (!PSY::WiFiManager::getTimeInitialized()) 
        {
            if (retryCount >= MAX_RETRIES) 
            {
                triggerError(
                    static_cast<uint32_t>(RoverTypes::StartupErrorCode::TIME_SYNC_FAILED),
                    "Failed to sync time",
                    ErrorType::WARNING
                );
                setState(RoverTypes::BehaviorState::HOME);
                return;
            }
            PSY::WiFiManager::syncTime();
            retryCount++;
        } 
        else 
        {
            setState(RoverTypes::BehaviorState::HOME);
            retryCount = 0;
        }
    }

    void RoverBehaviorManager::triggerFatalError(uint32_t errorCode, const char* errorMessage) {
        setState(RoverTypes::BehaviorState::FATAL_ERROR);
        RoverViewManager::errorCode = errorCode;
        RoverViewManager::errorMessage = errorMessage;
        RoverViewManager::isError = true;
        RoverViewManager::isFatalError = true;
        
        // Draw error screen
        RoverViewManager::drawErrorScreen(errorCode, errorMessage, true);
    }

    void RoverBehaviorManager::triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type) 
    {
        // Always log to serial
        Serial.printf("ERROR 0x%08X: %s (Type: %s)\n", 
            errorCode, 
            errorMessage, 
            type == ErrorType::FATAL ? "FATAL" : 
            type == ErrorType::WARNING ? "WARNING" : "SILENT"
        );
        
        if (type == ErrorType::SILENT) return;
        
        if (type == ErrorType::FATAL) 
        {
            setState(RoverTypes::BehaviorState::FATAL_ERROR);
        } 
        else 
        {
            setState(RoverTypes::BehaviorState::WARNING);
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

    void RoverBehaviorManager::setLoadingPhase(RoverTypes::LoadingPhase phase) 
    {
        loadingPhase = phase;
        
        switch(phase) 
        {
            case RoverTypes::LoadingPhase::BOOTING:
                currentStatusMessage = "Booting...";
                break;
            case RoverTypes::LoadingPhase::CONNECTING_WIFI:
                currentStatusMessage = "Connecting to WiFi...";
                break;
            case RoverTypes::LoadingPhase::SYNCING_TIME:
                currentStatusMessage = "Syncing time...";
                break;
        }
    }

    RoverTypes::LoadingPhase RoverBehaviorManager::getLoadingPhase() 
    {
        return loadingPhase;
    }

    const char* RoverBehaviorManager::getStatusMessage() 
    {
        return currentStatusMessage;
    }

}