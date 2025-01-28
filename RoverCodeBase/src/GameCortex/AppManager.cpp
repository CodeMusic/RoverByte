#include "AppManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../SomatosensoryCortex/MenuManager.h"
#include "../GameCortex/AppRegistration.h"

namespace GameCortex
{
    // Static member definitions
    bool AppManager::initialized = false;
    std::vector<AppManager::AppInfo> AppManager::appRegistry;
    AppManager::AppInfo* AppManager::activeApp = nullptr;
    AppManager::AppState AppManager::currentState = AppManager::AppState::IDLE;

    void AppManager::init() {
        if (initialized) {
            Utilities::LOG_DEBUG("AppManager already initialized");
            return;
        }
        
        try {
            appRegistry.clear();
            activeApp = nullptr;
            currentState = AppState::IDLE;
            initialized = true;
            Utilities::LOG_DEBUG("AppManager initialized successfully");
        } catch (const std::exception& e) {
            Utilities::LOG_ERROR("AppManager init failed: %s", e.what());
            initialized = false;
            throw;
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
        for (auto& app : appRegistry) {
            if (app.name == appName) {
                activeApp = &app;
                currentState = AppState::SHOW_INFO;
                // (Optional) immediately call onRun here
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
            // Display the quick info screen (app description)
            // After some user acknowledgment or short delay, switch to RUNNING
            // For demonstration, we automatically switch to RUNNING:
            currentState = AppState::RUNNING;
            break;

        case AppState::RUNNING:
            // Call the app's update function
            if (activeApp->onUpdate) {
                activeApp->onUpdate();
            }
            break;

        case AppState::IDLE:
        default:
            // No app running
            break;
        }
    }

    void AppManager::handleRotaryTurn(int direction) {
        // Only pass rotary events to the app if it's running
        if (currentState == AppState::RUNNING && activeApp && activeApp->onUpdate) {
            Utilities::LOG_DEBUG("Rotary turn: %d", direction);
            if (activeApp->name == "SlotsApp") {
                SlotsManager::handleRotaryTurn(direction);
            } else if (activeApp->name == "IrBlastApp") {
                IRManager::handleRotaryTurn(direction);
            } else if (activeApp->name == "NfcApp") {
                NFCManager::handleRotaryTurn(direction);
            }
            // If your app wants to handle left/right turning, you can pass it
            // For now, just call onUpdate again or do nothing.
            // This is up to you how you want to structure passing inputs to an app.
        }
    }

    void AppManager::handleRotaryPress() {
        // Only pass rotary button events to the app if it's running
        if (currentState == AppState::RUNNING && activeApp && activeApp->onUpdate) {
            if (activeApp->name == "SlotsApp") {
                SlotsManager::handleRotaryPress();
            } else if (activeApp->name == "IrBlastApp") {
                IRManager::handleRotaryPress();
            } else if (activeApp->name == "NfcApp") {
                NFCManager::handleRotaryPress();
            }
        }
    }

    void AppManager::exitApp() {
        if (activeApp && activeApp->onExit) {
            activeApp->onExit();
        }
        activeApp = nullptr;
        currentState = AppState::IDLE;
    }

    bool AppManager::isAppActive() {
        return (currentState != AppState::IDLE && activeApp != nullptr);
    }

    AppManager::AppState AppManager::getCurrentState() {
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