#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "../PrefrontalCortex/RoverBehaviorManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace PsychicCortex 
{
    using namespace CorpusCallosum;
    using PC::NetworkTypes::NetworkCredentials;

    class WiFiManager 
    {
    public:
        static bool init();
        static void checkConnection();
        static bool isConnected() { return isWiFiConnected; }
        static bool connectToWiFi();
        static bool getTimeInitialized() { return timeInitialized; }
        static bool syncTime();

    private:
        static constexpr unsigned long WIFI_RETRY_INTERVAL = 60000;
        static constexpr unsigned long TIME_CHECK_INTERVAL = 500;
        static constexpr unsigned long RESYNC_TIME_INTERVAL = 86400000;
        static constexpr const char* NTP_SERVER = "pool.ntp.org";
        static constexpr int32_t GMT_OFFSET_SEC = -5 * 3600;
        static constexpr int32_t DAY_LIGHT_OFFSET_SEC = 3600;

        static bool isWiFiConnected;
        static unsigned long lastWiFiAttempt;
        static bool timeInitialized;
        static unsigned long lastTimeCheck;

        static constexpr NetworkCredentials PRIMARY_NETWORK = 
        {
            "RevivalNetwork",
            "xunjmq84"
        };
        
        static constexpr NetworkCredentials BACKUP_NETWORK = 
        {
            "CodeMusicai",
            "cnatural"
        };
    };
}

#endif 