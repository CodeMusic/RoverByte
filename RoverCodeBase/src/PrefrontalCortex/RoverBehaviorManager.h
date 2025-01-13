#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_MANAGER_H

class RoverBehaviorManager {
public:
    enum BehaviorState {
        BOOTING,
        CONNECTING_WIFI,
        SYNCING_TIME,
        READY,
        ERROR
    };
    
    static void init();
    static void update();
    static BehaviorState getCurrentState() { return currentState; }
    static void setState(BehaviorState state);
    static const char* getStatusMessage() { return currentStatusMessage; }

private:
    static BehaviorState currentState;
    static const char* currentStatusMessage;
    static void handleBooting();
    static void handleWiFiConnection();
    static void handleTimeSync();
    static void handleError();
    static void handleReady();
};

#endif 