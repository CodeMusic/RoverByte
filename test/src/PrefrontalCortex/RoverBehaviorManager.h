#ifndef ROVER_BEHAVIOR_MANAGER_H
#define ROVER_BEHAVIOR_H

#include "../CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;  // Keep this for other utilities
    
    class RoverBehaviorManager 
    {
    public:
        static void init();
        static void update();
        static RoverTypes::BehaviorState getCurrentState();
        static void setState(RoverTypes::BehaviorState state);
        static RoverTypes::LoadingPhase getLoadingPhase();
        static void setLoadingPhase(RoverTypes::LoadingPhase phase);
        static const char* getStatusMessage();
        static void triggerFatalError(uint32_t errorCode, const char* errorMessage);
        static void handleFatalError();
        static void triggerError(uint32_t errorCode, const char* errorMessage, ErrorType type);
        static void handleWarning();
        static bool IsInitialized();
        static bool isErrorFatal(uint32_t errorCode);
        static int getCurrentBootStep();
        static void updateWarningCountdown();
        static constexpr unsigned long WARNING_DURATION = 120000;  // 2 minutes
        static constexpr unsigned long FATAL_REBOOT_DELAY = 60000;  // 1 minute
        static bool isWarningCountdownActive() { return isCountingDown; }
        static int getRemainingWarningSeconds();
        static unsigned long getWarningStartTime() { return warningStartTime; }

    private:
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

        static void handleLoading();
        static void handleHome();
        static void handleMenu();
        static void handleApp();
        static void handleError();
        static void handleBooting();
        static void handleWiFiConnection();
        static void handleTimeSync();
        static bool initialized;
    };
}

#endif // ROVER_BEHAVIOR_H 
