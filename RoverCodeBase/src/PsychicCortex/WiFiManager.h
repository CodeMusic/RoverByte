#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "../VisualCortex/RoverManager.h"
#include "../PrefrontalCortex/utilities.h"

// Forward declaration
class RoverViewManager;

#define GMT_OFFSET_SEC (-7 * 3600)  // PST
#define DAY_LIGHT_OFFSET_SEC 3600   // 1 hour of daylight saving
#define NTP_SERVER "pool.ntp.org"

class WiFiManager {
public:
    static void init();
    static void checkConnection();
    static bool isConnected() { return isWiFiConnected; }
    static void setCredentials(const char* primary_ssid, const char* primary_pass,
                             const char* backup_ssid, const char* backup_pass);
    static void connectToWiFi();
    static bool getWiFiStatus() { return isWiFiConnected; }
    static bool getTimeInitialized();

    static bool timeInitialized;
    
private:
    static bool isRecording;
    static bool isWiFiConnected;
    static unsigned long lastWiFiAttempt;
    static const unsigned long WIFI_RETRY_INTERVAL = 300000; // 5 minutes
    static const char* primarySSID;
    static const char* primaryPassword;
    static const char* backupSSID;
    static const char* backupPassword;
};

#endif 