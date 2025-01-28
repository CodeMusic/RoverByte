#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H
#include "utilities.h"
#include <Arduino.h>
#include <XPowersLib.h>
#include <FastLED.h>
#include "TFT_eSPI.h"

namespace PrefrontalCortex 
{

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

        // Timeout constants
        static const unsigned long IDLE_TIMEOUT = 60000;    // Base timeout (1 minute)
        static const unsigned long DIM_TIMEOUT = IDLE_TIMEOUT * 2;
        static const unsigned long SLEEP_TIMEOUT = IDLE_TIMEOUT * 3;
        static const uint8_t DIM_BRIGHTNESS = 128;         // 50% brightness

        // Add PWM constants
        static const uint8_t PWM_CHANNEL = 0;
        static const uint32_t PWM_FREQUENCY = 5000;
        static const uint8_t PWM_RESOLUTION = 8;
        static const uint8_t BACKLIGHT_PIN = 38;  // Verify this pin number
        
        // Core methods
        static void init();
        static void update();
        static void wakeFromSleep();
        static void enterDeepSleep();  // This is our primary sleep method

        static unsigned long getUpTime();
        
        // State getters/setters
        static SleepState getCurrentSleepState();
        static void updateLastActivityTime();
        
        // Display control
        static void setBacklight(uint8_t brightness);
        static void setupBacklight();
        
        // Battery management
        static int getBatteryPercentage();
        static String getChargeStatus();
        static bool isCharging();

        // Add missing method declarations
        static void initializeBattery();
        static int calculateBatteryPercentage(int voltage);
        static void checkSleepState();

    private:
        static XPowersPPM PPM;
        static bool batteryInitialized;
        static unsigned long lastActivityTime;
        static SleepState currentSleepState;
        static unsigned long startUpTime;
    };

}

#endif
