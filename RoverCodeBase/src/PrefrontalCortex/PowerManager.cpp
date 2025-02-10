#include "PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias
    using VC::LEDManager;
    using VC::RoverViewManager;
    using VC::RoverManager;

    // Initialize neural state variables
    XPowersPPM PowerManager::PPM;
    bool PowerManager::batteryInitialized = false;
    unsigned long PowerManager::lastActivityTime = 0;
    PowerState PowerManager::currentPowerState = PowerState::AWAKE;
    unsigned long PowerManager::startUpTime = 0;

    void PowerManager::init() 
    {
        startUpTime = millis();
        
        // Initialize sensory pathways
        pinMode(BOARD_PWR_EN, OUTPUT);
        digitalWrite(BOARD_PWR_EN, HIGH);
        
        setupBacklight();
        
        // Initialize metabolic regulation
        Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
        Wire.setClock(100000);
        
        delay(100);  // Neural stabilization period
        
        // Initialize homeostatic systems
        bool result = PPM.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, BQ25896_SLAVE_ADDRESS);
        if (result) 
        {
            batteryInitialized = true;
            Utilities::LOG_PROD("Metabolic regulation initialized");
            
            // Configure energy parameters
            configureEnergySystem();
        } 
        else 
        {
            Utilities::LOG_ERROR("Metabolic regulation initialization failed");
        }
        
        lastActivityTime = millis();
        currentPowerState = PowerState::AWAKE;
    }

    void PowerManager::configureEnergySystem() 
    {
        PPM.setSysPowerDownVoltage(3300);
        PPM.setInputCurrentLimit(3250);
        PPM.disableCurrentLimitPin();
        PPM.setChargeTargetVoltage(4208);
        PPM.setPrechargeCurr(64);
        PPM.setChargerConstantCurr(832);
        PPM.enableADCMeasure();
        PPM.enableCharge();
    }

    unsigned long PowerManager::getUpTime() {
        return (millis() - startUpTime) / 1000;  // Return uptime in seconds
    }

    void PowerManager::initializeBattery() {
        if (!batteryInitialized) {
            Utilities::LOG_DEBUG("Initializing battery management...");
            
            if (!PPM.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
                Utilities::LOG_ERROR("Failed to initialize battery management");
                return;
            }
            
            // Configure battery parameters
            PPM.setSysPowerDownVoltage(3300);
            PPM.setInputCurrentLimit(3250);
            PPM.disableCurrentLimitPin();
            PPM.setChargeTargetVoltage(4208);
            PPM.setPrechargeCurr(64);
            PPM.setChargerConstantCurr(832);
            PPM.enableADCMeasure();
            PPM.enableCharge();
            
            batteryInitialized = true;
            
            // Detailed logging
            Utilities::LOG_DEBUG("Battery configuration complete:");
            Utilities::LOG_DEBUG("- System power down voltage: %d", PPM.getSysPowerDownVoltage());
            Utilities::LOG_DEBUG("- Input current limit: %d", PPM.getInputCurrentLimit());
            Utilities::LOG_DEBUG("- Charge target voltage: %d", PPM.getChargeTargetVoltage());
            Utilities::LOG_DEBUG("- Precharge current: %d", PPM.getPrechargeCurr());
            Utilities::LOG_DEBUG("- Constant current: %d", PPM.getChargerConstantCurr());
            
            Utilities::LOG_PROD("Battery management initialized successfully");
        }
    }

    int PowerManager::calculateBatteryPercentage(int voltage) {
        const int MIN_VOLTAGE = 3300; // 3.3V
        const int MAX_VOLTAGE = 4200; // 4.2V
        
        if (voltage <= MIN_VOLTAGE) return 0;
        if (voltage >= MAX_VOLTAGE) return 100;
        
        return ((voltage - MIN_VOLTAGE) * 100) / (MAX_VOLTAGE - MIN_VOLTAGE);
    }

    void PowerManager::checkPowerState() {
        unsigned long idleTime = millis() - lastActivityTime;
        PowerState newState = currentPowerState;

        // State transitions
        if (idleTime < IDLE_TIMEOUT) {
            newState = PowerState::AWAKE;
        } else if (idleTime < IDLE_TIMEOUT * 2) {
            newState = PowerState::DIM_DISPLAY;
        } else if (idleTime < IDLE_TIMEOUT * 3) {
            newState = PowerState::DISPLAY_OFF;
        } else {
            newState = PowerState::DEEP_SLEEP;
        }

        // Only handle state change if needed
        if (newState != currentPowerState) {
            // Production level logging for state changes
            Utilities::LOG_PROD("Sleep state changing from %d to %d", currentPowerState, newState);
            currentPowerState = newState;
        }
    }

    void PowerManager::wakeFromSleep() {
        Utilities::LOG_PROD("Waking from sleep mode");
        lastActivityTime = millis();
        currentPowerState = PowerState::AWAKE;
        RoverManager::setShowTime(false);
        
        // Initialize display first
        tft.init();
        tft.writecommand(TFT_SLPOUT);
        delay(120);
        tft.writecommand(TFT_DISPON);
        
        // Initialize LED system
        LEDManager::init();
        
        // Restore backlight
        setBacklight(255);
        
        // Re-initialize critical components
        RoverViewManager::init();
        
        // Force display update
        drawSprite();
    }

    PowerState PowerManager::getCurrentPowerState() {
        return currentPowerState;
    }

    int PowerManager::getBatteryPercentage() {
        if (!batteryInitialized) return 0;
        
        int voltage = PPM.getBattVoltage();
        return calculateBatteryPercentage(voltage);
    }

    String PowerManager::getChargeStatus() {
        if (!batteryInitialized) return "Not Found";
        return PPM.getChargeStatusString();
    }

    bool PowerManager::isCharging() {
        if (!batteryInitialized) return false;
        return PPM.isVbusIn();
    }

    void PowerManager::updateLastActivityTime() {
        lastActivityTime = millis();
    }

    void PowerManager::enterDeepSleep() {
        LEDManager::stopLoadingAnimation();

        FastLED.show();
        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(TFT_SLPIN);
        
        // Configure wake-up sources with correct GPIO pins
        gpio_pullup_en(GPIO_NUM_0);    // Enable pull-up on BOARD_USER_KEY
        gpio_pullup_en(GPIO_NUM_21);   // Enable pull-up on ENCODER_KEY
        
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);    // Wake on BOARD_USER_KEY falling edge
        esp_sleep_enable_ext1_wakeup(1ULL << GPIO_NUM_21, ESP_EXT1_WAKEUP_ANY_HIGH);  // Wake on ENCODER_KEY
        
        // Wait for buttons to be released
        while (digitalRead(BOARD_USER_KEY) == LOW || digitalRead(ENCODER_KEY) == LOW) {
            delay(10);
        }
        delay(100);
        
        esp_deep_sleep_start();
    }

    void PowerManager::setupBacklight() {
        ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
        ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL);
        ledcWrite(PWM_CHANNEL, 255);  // Full brightness
        Serial.println("Backlight setup complete");
    }

    void PowerManager::setBacklight(uint8_t brightness) {
        ledcWrite(PWM_CHANNEL, brightness);
        drawSprite();
    }

    void PowerManager::update() {
        unsigned long currentTime = millis();
        PowerState newState = currentPowerState;  // Changed from SleepState to PowerState
        
        // Debug level logging for idle time
        unsigned long idleTime = currentTime - lastActivityTime;
        Utilities::LOG_DEBUG("Idle time: %lu ms, Current state: %d", idleTime, currentPowerState);
        
        // State transitions
        if (idleTime < IDLE_TIMEOUT) {
            newState = PowerState::AWAKE;
        } else if (idleTime < DIM_TIMEOUT) {
            newState = PowerState::DIM_DISPLAY;
        } else if (idleTime < SLEEP_TIMEOUT) {
            newState = PowerState::DISPLAY_OFF;
        } else {
            newState = PowerState::DEEP_SLEEP;
        }
        
        // Only handle state change if needed
        if (newState != currentPowerState) {
            Utilities::LOG_PROD("Power state changing from %d to %d", currentPowerState, newState);
            
            // Handle state-specific actions
            switch (newState) {
                case PowerState::DIM_DISPLAY:
                    setBacklight(DIM_BRIGHTNESS);
                    break;
                    
                case PowerState::DISPLAY_OFF:
                    setBacklight(0);
                    break;
                    
                case PowerState::DEEP_SLEEP:
                    enterDeepSleep();
                    break;
                    
                case PowerState::AWAKE:
                    setBacklight(255);
                    break;
            }
            
            currentPowerState = newState;
        }
    }

}

