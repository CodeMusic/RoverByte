#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_MANAGER_H

#include "PrefrontalCortex/Utilities.h"
#include "CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;  // Keep this for other utilities
    
    /**
     * @brief Manages high-level cognitive behaviors and state transitions
     * 
     * Controls:
     * - Executive function state management
     * - Behavioral response patterns
     * - Error detection and recovery
     * - Cognitive initialization sequence
     * - Temporal processing of warnings
     */
    class RoverBehaviorManager 
    {
    public:
        // Temporal processing constants
        static constexpr unsigned long WARNING_DURATION = 120000;  // 2 minutes
        static constexpr unsigned long FATAL_REBOOT_DELAY = 60000;  // 1 minute

        // Core cognitive functions
        static void init();
        static void update();
        static RoverTypes::BehaviorState getCurrentState();
        static void setState(RoverTypes::BehaviorState state);
        static bool isInitialized();
        static bool isValid();
        static void attemptRecovery();

        // Executive function processing
        static RoverTypes::LoadingPhase getLoadingPhase();
        static void setLoadingPhase(RoverTypes::LoadingPhase phase);
        static const char* getStatusMessage();
        static int getCurrentBootStep();

        // Error detection and recovery
        static void triggerFatalError(uint32_t errorCode, const char* errorMessage);
        static void handleFatalError();
        static void triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type);
        static void handleWarning();
        static bool isErrorFatal(uint32_t errorCode);

        // Warning temporal processing
        static void updateWarningCountdown();
        static bool isWarningCountdownActive();
        static int getRemainingWarningSeconds();
        static unsigned long getWarningStartTime();

    private:
        // Neural state variables
        static RoverTypes::BehaviorState currentState;
        static RoverTypes::LoadingPhase loadingPhase;
        static const char* currentStatusMessage;
        static RoverTypes::BehaviorState previousState;
        static unsigned long warningStartTime;
        static bool isCountingDown;
        static int currentBootStep;
        static unsigned long fatalErrorStartTime;
        static bool isFatalError;
        static VisualTypes::VisualPattern pattern;
        static String statusMessage;
        static bool initialized;

        // Behavioral processing methods
        static void handleLoading();
        static void handleHome();
        static void handleMenu();
        static void handleApp();
        static void handleError();
        static void handleBooting();
        static void handleWiFiConnection();
        static void handleTimeSync();
    };
}

#endif // ROVER_BEHAVIOR_MANAGER_H 
