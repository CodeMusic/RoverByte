#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <XPowersLib.h>
#include <FastLED.h>
#include "TFT_eSPI.h"
#include "utilities.h"

// Forward declarations
extern TFT_eSPI tft;
extern void setBacklight(uint8_t brightness);
extern void drawSprite();

class PowerManager {
public:
    enum SleepState {
        AWAKE,
        DIM_DISPLAY,    
        DISPLAY_OFF,    
        DEEP_SLEEP      
    };

    static void init();
    static void checkSleepState();
    static void wakeFromSleep();
    static void enterDeepSleep();
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
    static const unsigned long IDLE_TIMEOUT = 60000;
    
    static int calculateBatteryPercentage(int voltage);
    static void initializeBattery();
};

#endif