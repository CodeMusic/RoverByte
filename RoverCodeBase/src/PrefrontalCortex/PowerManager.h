#pragma once
#include "CorpusCallosum/SynapticPathways.h"
#include "PrefrontalCortex/ProtoPerceptions.h"
#include "PrefrontalCortex/Utilities.h"
#include <Arduino.h>
#include <XPowersLib.h>
#include <FastLED.h>
#include "TFT_eSPI.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;
    using PC::PowerTypes::PowerState;
    using PC::PowerTypes::BatteryStatus;
    using PC::PowerTypes::PowerConfig;

    /**
     * @brief Manages cognitive energy states and metabolic processes
     * 
     * Controls:
     * - Energy state transitions (wake/sleep cycles)
     * - Metabolic rate regulation (power consumption)
     * - Homeostatic maintenance (battery management)
     * - Circadian rhythm synchronization (display brightness)
     */
    class PowerManager 
    {
    public:
        // Circadian rhythm constants
        static const unsigned long IDLE_TIMEOUT = 60000;    // Base timeout (1 minute)
        static const unsigned long DIM_TIMEOUT = IDLE_TIMEOUT * 2;
        static const unsigned long SLEEP_TIMEOUT = IDLE_TIMEOUT * 3;
        
        // Sensory perception thresholds
        static const uint8_t DIM_BRIGHTNESS = 128;         // 50% brightness
        static const uint8_t PWM_CHANNEL = 0;
        static const uint32_t PWM_FREQUENCY = 5000;
        static const uint8_t PWM_RESOLUTION = 8;
        static const uint8_t BACKLIGHT_PIN = 38;

        // Core cognitive functions
        static void init();
        static void update();
        static void wakeFromSleep();
        static void enterDeepSleep();
        static unsigned long getUpTime();
        
        // Homeostatic regulation
        static PowerState getCurrentPowerState();
        static void updateLastActivityTime();
        static void setBacklight(uint8_t brightness);
        static void setupBacklight();
        
        // Metabolic monitoring
        static BatteryStatus getBatteryStatus();
        static String getChargeStatus();
        static bool isCharging();
        static void initializeBattery();
        static int calculateBatteryPercentage(int voltage);
        static void checkPowerState();
        static int getBatteryPercentage();
        static void configureEnergySystem();

    private:
        // Neural state variables
        static XPowersPPM PPM;
        static bool batteryInitialized;
        static unsigned long lastActivityTime;
        static PowerState currentPowerState;
        static unsigned long startUpTime;
    };

    // External neural pathways
    extern TFT_eSPI tft;
    extern void setBacklight(uint8_t brightness);
    extern void drawSprite();
}
