#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H
#include "PowerManager.h" 
#include "utilities.h"
#include <Arduino.h>
#include <XPowersLib.h>
#include <FastLED.h>
#include "TFT_eSPI.h"

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
    static void setBacklight(uint8_t brightness);
    static void setupBacklight();
    static void setShowTime(bool show);
    static bool getShowTime();
    
private:
    static const uint8_t BACKLIGHT_PIN = 38;
    static const uint8_t PWM_CHANNEL = 0;
    static const uint8_t PWM_RESOLUTION = 8;
    static const uint32_t PWM_FREQUENCY = 5000;

    static XPowersPPM PPM;
    static bool batteryInitialized;
    static unsigned long lastActivityTime;
    static SleepState currentSleepState;
    static const unsigned long IDLE_TIMEOUT = 60000;
    
    static int calculateBatteryPercentage(int voltage);
    static void initializeBattery();
    static bool showTime;
};
#endif
