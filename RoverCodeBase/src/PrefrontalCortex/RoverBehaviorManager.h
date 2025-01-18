#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_MANAGER_H

// Forward declaration
class WiFiManager;

class RoverBehaviorManager {
public:
    // Enum declarations
    enum class BehaviorState {
        LOADING,
        HOME,
        MENU,
        APP,
        ERROR
    };

    enum class LoadingPhase {
        BOOTING,
        CONNECTING_WIFI,
        SYNCING_TIME
    };

    // Static methods
    static void init();
    static void update();
    
    static BehaviorState getCurrentState();
    static void setState(BehaviorState state);

    static LoadingPhase getLoadingPhase();
    static void setLoadingPhase(LoadingPhase phase);

    static const char* getStatusMessage();

private:
    static BehaviorState currentState;
    static LoadingPhase loadingPhase;
    static const char* currentStatusMessage;

    // Main handlers for each top-level state
    static void handleLoading();
    static void handleHome();
    static void handleMenu();
    static void handleApp();
    static void handleError();

    // Sub-phase handlers for LOADING
    static void handleBooting();
    static void handleWiFiConnection();
    static void handleTimeSync();
};

#endif 