#include "AppManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../SomatosensoryCortex/MenuManager.h"

// Static member definitions
std::vector<AppManager::AppInfo> AppManager::appRegistry;
AppManager::AppInfo* AppManager::activeApp = nullptr;
AppManager::AppState AppManager::currentState = AppManager::AppState::IDLE;

void AppManager::init() {
    // Optionally load or register some default apps
    // e.g., NFC scanning app, Slots, etc.
}

void AppManager::registerApp(const AppInfo& app) {
    appRegistry.push_back(app);
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
        // If your app wants to handle left/right turning, you can pass it
        // For now, just call onUpdate again or do nothing.
        // This is up to you how you want to structure passing inputs to an app.
    }
}

void AppManager::handleRotaryPress() {
    // Only pass rotary button events to the app if it's running
    if (currentState == AppState::RUNNING && activeApp && activeApp->onUpdate) {
        // Similarly, app can handle "press to do something"
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