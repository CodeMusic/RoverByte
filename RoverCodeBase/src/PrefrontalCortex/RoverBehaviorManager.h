#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_MANAGER_H

class RoverBehaviorManager {
public:
    // Top-level states
    enum BehaviorState {
        LOADING,
        HOME,
        MENU,
        APP,
        ERROR
    };

    // Sub-phases (used only when currentState == LOADING)
    enum LoadingPhase {
        BOOTING,
        CONNECTING_WIFI,
        SYNCING_TIME
    };

    static void init();
    static void update();
    
    static BehaviorState getCurrentState();
    static void setState(BehaviorState state);

    // Sub-phase getters/setters for LOADING
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