#pragma once

#include <vector>
#include <functional>
#include <string>

namespace GameCortex
{
    class AppManager {
    public:
        // Represents the overall state of the active app
        enum class AppState {
            IDLE,       // No app is active
            SHOW_INFO,  // Displaying a quick info screen
            RUNNING     // The app is currently running
        };

        // Holds metadata and function pointers for each app
        struct AppInfo {
            std::string name;
            std::string description;
            // Called once when app "starts"
            void (*onRun)();
            // Called repeatedly while app is running (e.g., per update cycle)
            void (*onUpdate)();
            // Called when the user selects "Exit" from the app menu
            void (*onExit)();
        };

        // Initialize the app manager
        static void init();
        static bool isInitialized() { return initialized; }

        // Register a new app
        static bool registerApp(const AppInfo& app);

        // Start an app by name (triggers onRun, switches to SHOW_INFO state initially)
        static bool startApp(const std::string& appName);

        // Called in the main loop to update the active app
        static void update();

        // Handle rotary inputs if the app is active
        static void handleRotaryTurn(int direction);
        static void handleRotaryPress();

        // Exit current app
        static void exitApp();

        // Is there an app running now?
        static bool isAppActive();

        // Returns the current state (IDLE, SHOW_INFO, RUNNING)
        static AppState getCurrentState();

        // Name/description of the currently running app
        static std::string getCurrentAppName();
        static std::string getCurrentAppDescription();

    private:
        static bool initialized;
        static std::vector<AppInfo> appRegistry;
        static AppInfo* activeApp;
        static AppState currentState;
    }; 
}