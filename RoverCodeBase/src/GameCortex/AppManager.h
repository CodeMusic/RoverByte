/**
 * @brief AppManager handles the cognitive engagement and entertainment subsystems
 * 
 * This class manages the rover's application ecosystem, providing:
 * - Application lifecycle management (initialization, execution, termination)
 * - State transitions between DORMANT, ORIENTING, and ENGAGED states
 * - Input handling and cognitive routing for applications
 * - Cross-cortex coordination for app-specific behaviors
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "CorpusCallosum/SynapticPathways.h"
#include "PrefrontalCortex/ProtoPerceptions.h"
#include <vector>
#include <functional>
#include <string>

namespace GameCortex
{
    using namespace CorpusCallosum;
    using PC::GameTypes::AppInfo;
    using PC::GameTypes::AppState;

    class AppManager 
    {
    public:
        /**
         * @brief Initializes the application management system
         */
        static void init();

        /**
         * @brief Checks if the manager has been initialized
         * @return True if initialized, false otherwise
         */
        static bool isInitialized() { return initialized; }

        /**
         * @brief Registers a new application in the cognitive registry
         * @param app The application information to register
         * @return True if registration successful, false if duplicate or invalid
         */
        static bool registerApp(const AppInfo& app);

        /**
         * @brief Activates an application by transitioning it to ORIENTING state
         * @param appName The name of the application to start
         * @return True if application started successfully
         */
        static bool startApp(const std::string& appName);

        /**
         * @brief Updates the currently active application's cognitive state
         */
        static void update();

        /**
         * @brief Routes rotary encoder input to the active application
         * @param direction The direction of rotation (-1 or 1)
         */
        static void handleRotaryTurn(int direction);

        /**
         * @brief Routes rotary encoder button press to the active application
         */
        static void handleRotaryPress();

        /**
         * @brief Terminates the current application and returns to DORMANT state
         */
        static void exitApp();

        /**
         * @brief Checks if an application is currently in ORIENTING or ENGAGED state
         * @return True if an application is active
         */
        static bool isAppActive();

        /**
         * @brief Gets the current cognitive state of the application system
         * @return Current AppState (DORMANT, ORIENTING, or ENGAGED)
         */
        static AppState getCurrentState();

        /**
         * @brief Gets the name of the currently active application
         * @return Application name or empty string if none active
         */
        static std::string getCurrentAppName();

        /**
         * @brief Gets the description of the currently active application
         * @return Application description or empty string if none active
         */
        static std::string getCurrentAppDescription();

    private:
        static bool initialized;
        static std::vector<AppInfo> appRegistry;
        static AppInfo* activeApp;
        static AppState currentState;
    }; 
}

#endif // APP_MANAGER_H