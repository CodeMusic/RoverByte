#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

// Forward declaration
class RoverViewManager;

#define GMT_OFFSET_SEC (-5 * 3600)  // EST (DST)
#define DAY_LIGHT_OFFSET_SEC 3600   // 1 hour of daylight saving
#define NTP_SERVER "pool.ntp.org"

class WiFiManager {
public:
    static void init();
    static void checkConnection();
    static bool isConnected() { return isWiFiConnected; }
    static void connectToWiFi();
    static bool getTimeInitialized() { return timeInitialized; }
    static void syncTime();

private:
    static bool isRecording;
    static bool isWiFiConnected;
    static unsigned long lastWiFiAttempt;
    static bool timeInitialized;
    static unsigned long lastTimeCheck;
    // WiFi credentials
    static constexpr const char* PRIMARY_SSID = "RevivalNetwork ";
    static constexpr const char* PRIMARY_PASSWORD = "xunjmq84";
    static constexpr const char* BACKUP_SSID = "CodeMusicai";
    static constexpr const char* BACKUP_PASSWORD = "cnatural";
};

#endif 