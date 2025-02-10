/**
 * @file AppManager.cpp
 * @brief Implementation of the AppManager class for cognitive engagement systems
 * 
 * Handles the implementation of application lifecycle management and state transitions
 * for the entertainment and engagement subsystems.
 */

#include "AppManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "AppRegistration.h"
#include "SlotsManager.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../PsychicCortex/IRManager.h"
#include "../PsychicCortex/NFCManager.h"

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::AppInfo;
    using PC::GameTypes::AppState;
    using PC::Utilities;
    using PSY::IRManager;
    using PSY::NFCManager;

    // Static member definitions
    bool AppManager::initialized = false;
    std::vector<AppInfo> AppManager::appRegistry;
    AppInfo* AppManager::activeApp = nullptr;
    AppState AppManager::currentState = AppState::IDLE;

    void AppManager::init() {
        if (!initialized) {
            appRegistry.clear();
            activeApp = nullptr;
            currentState = AppState::IDLE;
            initialized = true;
        }
    }

    bool AppManager::registerApp(const AppInfo& app) {
        if (!initialized) {
            Utilities::LOG_ERROR("Cannot register app - AppManager not initialized");
            return false;
        }

        try {
            if (app.name.empty()) {
                Utilities::LOG_ERROR("Invalid app registration - empty name");
                return false;
            }
            appRegistry.push_back(app);
            Utilities::LOG_DEBUG("Registered app: %s", app.name.c_str());
            return true;
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("App registration failed: %s", e.what());
            return false;
        }
    }

    bool AppManager::startApp(const std::string& appName) {
        // First call onExit for any currently running app
        if (activeApp && activeApp->onExit) {
            activeApp->onExit();
        }

        for (auto& app : appRegistry) {
            if (app.name == appName) {
                activeApp = &app;
                currentState = AppState::SHOW_INFO;
                // Call the onRun function if it exists
                if (activeApp->onRun) {
                    activeApp->onRun();
                }
                return true;
            }
        }
        return false;
    }

    void AppManager::update() {
        if (!activeApp) return;

        switch (currentState) {
        case AppState::SHOW_INFO:
            currentState = AppState::RUNNING;
            break;

        case AppState::RUNNING:
            // Call the app's onUpdate function if it exists
            if (activeApp->onUpdate) {
                activeApp->onUpdate();
            }
            break;

        case AppState::IDLE:
        default:
            break;
        }
    }

    void AppManager::handleRotaryTurn(int direction) {
        if (currentState == AppState::RUNNING && activeApp) {
            Utilities::LOG_DEBUG("Rotary turn: %d", direction);
            const std::string appName = activeApp->name;
            
            if (appName == "SlotsApp") {
                SlotsManager::handleRotaryTurn(direction);
            } else if (appName == "IrBlastApp") {
                IRManager::handleRotaryTurn(direction);
            } else if (appName == "NfcApp") {
                NFCManager::handleRotaryTurn(direction);
            }
        }
    }

    void AppManager::handleRotaryPress() {
        if (currentState == AppState::RUNNING && activeApp) {
            const std::string appName = activeApp->name;
            
            if (appName == "SlotsApp") {
                SlotsManager::handleRotaryPress();
            } else if (appName == "IrBlastApp") {
                IRManager::handleRotaryPress();
            } else if (appName == "NfcApp") {
                NFCManager::handleRotaryPress();
            }
        }
    }

    void AppManager::exitApp() {
        // Call onExit before resetting state
        if (activeApp && activeApp->onExit) {
            activeApp->onExit();
        }
        activeApp = nullptr;
        currentState = AppState::IDLE;
    }

    bool AppManager::isAppActive() {
        return (currentState != AppState::IDLE && activeApp != nullptr);
    }

    AppState AppManager::getCurrentState() {
        return currentState;
    }

    std::string AppManager::getCurrentAppName() {
        if (activeApp) return activeApp->name;
        return "";
    }

    std::string AppManager::getCurrentAppDescription() {
        if (activeApp) return activeApp->description;
        return "";
    } 
}