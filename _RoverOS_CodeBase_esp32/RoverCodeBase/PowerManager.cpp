#include "PowerManager.h"

// Initialize static members
XPowersPPM PowerManager::PPM;
bool PowerManager::batteryInitialized = false;
unsigned long PowerManager::lastActivityTime = 0;
PowerManager::SleepState PowerManager::currentSleepState = PowerManager::AWAKE;

void PowerManager::init() {
    initializeBattery();
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
}

PowerManager::SleepState PowerManager::getCurrentSleepState() {
    return currentSleepState;
}

int PowerManager::getBatteryPercentage() {
    if (!batteryInitialized) return 0;
    return calculateBatteryPercentage(PPM.getBattVoltage());
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