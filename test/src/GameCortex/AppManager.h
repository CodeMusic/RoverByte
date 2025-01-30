#pragma once

#include "../CorpusCallosum/SynapticPathways.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include <vector>
#include <functional>
#include <string>

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::AppInfo;
    using PC::GameTypes::AppState;

    class AppManager {
    public:
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

        // Returns the current state (DORMANT, ORIENTING, ENGAGED)
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