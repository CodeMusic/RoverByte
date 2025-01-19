#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_MANAGER_H

#include <stdint.h>  // Add this for uint32_t
#include "../SomatosensoryCortex/UIManager.h"  // Add this for UIManager

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
        WARNING,
        ERROR,
        FATAL_ERROR,  // New state for unrecoverable errors
        IDLE
    };

    enum class LoadingPhase {
        BOOTING,
        CONNECTING_WIFI,
        SYNCING_TIME
    };

    enum class StartupErrorCode : uint32_t {
        CORE_INIT_FAILED = 0x01,
        POWER_INIT_FAILED = 0x02,
        LED_INIT_FAILED = 0x03,
        DISPLAY_INIT_FAILED = 0x04,
        UI_INIT_FAILED = 0x05,
        MENU_INIT_FAILED = 0x06,
        AUDIO_INIT_FAILED = 0x07,
        STORAGE_INIT_FAILED = 0x08,
        WIFI_INIT_FAILED = 0x09,
        TIME_SYNC_FAILED = 0x0A
    };

    enum class ErrorType {
        WARNING,
        FATAL,
        SILENT  // New error type for serial-only output
    };

    struct ErrorInfo {
        uint32_t code;
        const char* message;
        ErrorType type;
    };

    // Static methods
    static void init();
    static void update();
    
    static BehaviorState getCurrentState();
    static void setState(BehaviorState state);

    static LoadingPhase getLoadingPhase();
    static void setLoadingPhase(LoadingPhase phase);

    static const char* getStatusMessage();

    static void triggerFatalError(uint32_t errorCode, const char* errorMessage);
    static void handleFatalError();

    static void triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type);
    static void handleWarning();

    static bool isErrorFatal(uint32_t errorCode);

    static int getCurrentBootStep();

    static void updateWarningCountdown();
    static bool isWarningCountdownActive() { return isCountingDown; }

    static const unsigned long WARNING_DURATION = 3000; // 3 seconds for warnings

    static unsigned long getWarningStartTime() { return warningStartTime; }

private:
    static BehaviorState currentState;
    static LoadingPhase loadingPhase;
    static const char* currentStatusMessage;
    static BehaviorState previousState;
    static unsigned long warningStartTime;
    static bool isCountingDown;
    static int currentBootStep;

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