#include "../PrefrontalCortex/PowerManager.h"
#include "../VisualCortex/LEDManager.h"
#include "../VisualCortex/RoverViewManager.h"

// Initialize static members
XPowersPPM PowerManager::PPM;
bool PowerManager::batteryInitialized = false;
unsigned long PowerManager::lastActivityTime = 0;
PowerManager::SleepState PowerManager::currentSleepState = PowerManager::AWAKE;

void PowerManager::init() {
    // Initialize LEDC for backlight first
    setupBacklight();
    
    // Initialize I2C for battery management
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    Wire.setClock(100000);  // Set I2C clock to 100kHz
    
    delay(100);  // Give I2C time to stabilize
    
    // Try to initialize battery with retries
    for (int attempts = 0; attempts < 3; attempts++) {
        if (PPM.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
            batteryInitialized = true;
            LOG_PROD("Battery management initialized successfully");
            
            // Configure battery parameters
            PPM.setSysPowerDownVoltage(3300);
            PPM.setInputCurrentLimit(3250);
            PPM.setChargeTargetVoltage(4208);
            PPM.enableADCMeasure();
            PPM.enableCharge();
            break;
        }
        delay(100);
    }
    
    lastActivityTime = millis();
    currentSleepState = AWAKE;
}

void PowerManager::initializeBattery() {
    LOG_DEBUG("Initializing battery management...");
    bool result = PPM.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, BQ25896_SLAVE_ADDRESS);
    if (result) {
        LOG_PROD("Battery management initialized successfully");
        PPM.setSysPowerDownVoltage(3300);
        PPM.setInputCurrentLimit(3250);
        PPM.disableCurrentLimitPin();
        PPM.setChargeTargetVoltage(4208);
        PPM.setPrechargeCurr(64);
        PPM.setChargerConstantCurr(832);
        PPM.enableADCMeasure();
        PPM.enableCharge();
        batteryInitialized = true;
        
        // Scope level logging for detailed configuration
        LOG_SCOPE("Battery configuration complete:");
        LOG_SCOPE("- System power down voltage: 3300mV");
        LOG_SCOPE("- Input current limit: 3250mA");
        LOG_SCOPE("- Charge target voltage: 4208mV");
        LOG_SCOPE("- Precharge current: 64mA");
        LOG_SCOPE("- Constant current: 832mA");
    } else {
        LOG_PROD("Failed to initialize battery management");
    }
}

int PowerManager::calculateBatteryPercentage(int voltage) {
    const int maxVoltage = 4200; // 4.2V fully charged
    const int minVoltage = 3300; // 3.3V empty
    
    int percentage = map(voltage, minVoltage, maxVoltage, 0, 100);
    return constrain(percentage, 0, 100);
}

void PowerManager::checkSleepState() {
    unsigned long idleTime = millis() - lastActivityTime;
    SleepState newState = currentSleepState;

    // Debug level logging for idle time
    LOG_DEBUG("Idle time: %lu ms, Current state: %d", idleTime, currentSleepState);

    // State transitions
    if (idleTime < IDLE_TIMEOUT) {
        newState = AWAKE;
    } else if (idleTime < IDLE_TIMEOUT * 2) {
        newState = DIM_DISPLAY;
    } else if (idleTime < IDLE_TIMEOUT * 3) {
        newState = DISPLAY_OFF;
    } else {
        newState = DEEP_SLEEP;
    }

    // Only handle state change if needed
    if (newState != currentSleepState) {
        // Production level logging for state changes
        LOG_PROD("Sleep state changing from %d to %d", currentSleepState, newState);
        currentSleepState = newState;
    }
}

void PowerManager::wakeFromSleep() {
    LOG_PROD("Waking from sleep mode");
    lastActivityTime = millis();
    currentSleepState = AWAKE;
    
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

PowerManager::SleepState PowerManager::getCurrentSleepState() {
    return currentSleepState;
}

int PowerManager::getBatteryPercentage() {
    if (!batteryInitialized) return 0;
    float voltage = PPM.getBattVoltage();
    
    // Define the minimum and maximum voltage for your battery
    const float MIN_VOLTAGE = 3.3;  // Voltage at 0%
    const float MAX_VOLTAGE = 4.2;  // Voltage at 100%
    
    if (voltage <= MIN_VOLTAGE) return 0;
    if (voltage >= MAX_VOLTAGE) return 100;
    
    // Calculate the percentage based on the voltage range
    return (int)((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0);
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


void goToSleep() {
    // Ensure all pending operations are complete
    FastLED.clear(true);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    
    // Wait for any buttons to be released and debounce
    while (digitalRead(BOARD_USER_KEY) == LOW || digitalRead(ENCODER_KEY) == LOW) {
        delay(10);
    }
    delay(100);  // Additional debounce delay
    
    // Go to deep sleep - will wake on any configured button press
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