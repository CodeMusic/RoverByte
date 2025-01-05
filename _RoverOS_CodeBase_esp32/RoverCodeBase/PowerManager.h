#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <XPowersLib.h>
#include "utilities.h"

class PowerManager {
public:
    enum SleepState {
        AWAKE,
        DIM_DISPLAY,    // 50% brightness
        DISPLAY_OFF,    // Screen off, LEDs on
        DEEP_SLEEP      // Screen and LEDs off
    };

    static void init();
    static void checkSleepState();
    static void wakeFromSleep();
    static SleepState getCurrentSleepState();
    static int getBatteryPercentage();
    static String getChargeStatus();
    static bool isCharging();
    static void updateLastActivityTime();
    
private:
    static XPowersPPM PPM;
    static bool batteryInitialized;
    static unsigned long lastActivityTime;
    static SleepState currentSleepState;
    static const unsigned long IDLE_TIMEOUT = 60000;  // 60 seconds for each stage
    
    static int calculateBatteryPercentage(int voltage);
    static void initializeBattery();
};

#endif