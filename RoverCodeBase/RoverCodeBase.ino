#include <SPI.h>
#include <Wire.h>
#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <XPowersLib.h>
#include "driver/i2s.h"

#include "src/PrefrontalCortex/utilities.h"
#include "src/PrefrontalCortex/PowerManager.h"
#include "src/VisualCortex/RoverViewManager.h"
#include "src/VisualCortex/RoverManager.h"
#include "src/VisualCortex/ColorUtilities.h"
#include "src/VisualCortex/DisplayConfig.h"
#include "src/AuditoryCortex/SoundFxManager.h"
#include "src/VisualCortex/LEDManager.h"
#include "src/PrefrontalCortex/WiFiManager.h"
#include "src/PrefrontalCortex/SDManager.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define CLOCK_PIN 45

RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

int value=980;
bool muted=0;
int deb=0;


// Update WiFi credentials at top
const char* primary_ssid = "RevivalNetwork ";
const char* primary_password = "xunjmq84";
const char* backup_ssid = "CodeMusicai";
const char* backup_password = "cnatural";

#define SCREEN_CENTER_X 85  // Confirm this matches your screen width

// Add at top with other global variables
bool isLowBrightness = false;
bool showTime = false;

// Add at top with other global variables
unsigned long lastCounterUpdate = 0;
const unsigned long COUNTER_SPEED = 1000;  // 1 second interval
unsigned long lastAnimationStep = 0;
const unsigned long ANIMATION_DELAY = 250;  // 250ms between animation steps
bool isAnimating = false;
int animationStep = 0;
const int TOTAL_ANIMATION_STEPS = 14;  // 7 steps for turning off + 7 steps for turning on

static int pos = 0;


// Add at top with other global variables
unsigned long lastStatusUpdate = 0;
const unsigned long STATUS_CHANGE_INTERVAL = 3000;  // Switch every 3 seconds
bool showLevel = true;  // Toggle between level and experience


unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_RETRY_INTERVAL = 300000;  // Try every 5 minutes
bool isWiFiConnected = false;

// Add at top with other global variables
XPowersPPM PPM;
bool batteryInitialized = false;
int batteryPercentage = 0;
String chargeStatus = "Unknown";

// Add these definitions
#define RECORD_FILENAME "/sdcard/temp_record.wav"
#define EXAMPLE_I2S_CH      0
#define EXAMPLE_SAMPLE_RATE 44100
#define EXAMPLE_BIT_SAMPLE  16
#define NUM_CHANNELS        1
#define SAMPLE_SIZE         (EXAMPLE_BIT_SAMPLE * 1024)
#define BYTE_RATE          (EXAMPLE_SAMPLE_RATE * (EXAMPLE_BIT_SAMPLE / 8)) * NUM_CHANNELS

// Add these globals
static int16_t i2s_readraw_buff[SAMPLE_SIZE];
size_t bytes_read;
bool isRecording = false;
File recordFile;

// Add these near your other global variables
#define RECORD_BUFFER_SIZE 1024  // Smaller buffer size
static uint8_t recording_buffer[RECORD_BUFFER_SIZE];

// Add these globals near the top
const unsigned long DOUBLE_CLICK_TIME = 500;  // Maximum time between clicks (ms)
unsigned long lastButtonPress = 0;
bool isInSleepMode = false;

// Add these globals near the top
bool rotaryButtonPressed = false;
bool sideButtonPressed = false;

// Add these near your other global variables
const unsigned long IDLE_TIMEOUT = 60000;  // 60 seconds for each stage
unsigned long lastActivityTime = 0;
enum SleepState {
    AWAKE,
    DIM_DISPLAY,    // 50% brightness
    DISPLAY_OFF,    // Screen off, LEDs on
    DEEP_SLEEP      // Screen and LEDs off
} currentSleepState = AWAKE;


// Add these constants at the top with other defines
#define TONE_PWM_CHANNEL 2  // Use a different channel than the backlight
#define TONE_PIN BOARD_VOICE_DIN


// Add at the top with other globals
bool isDimmed = false;
const int NUM_LEDS = 8;  // 8 LEDs total

// Add near other global variables
unsigned long lastExpressionChange = 0;
unsigned long nextExpressionInterval = 60000;  // Start with 1 minute

// Add this near the top with other function declarations
void drawBatteryCharging(int x, int y, int size);



// Battery initialization function
void initializeBattery() {
    if (!batteryInitialized) {
        if (!PPM.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
            LOG_ERROR("Failed to initialize battery management");
            return;
        }
        batteryInitialized = true;
    }
}

// Audio instance
Audio audio;
bool isPlayingSound = false;

void handleSDCardOperation() {
    if (!SD.begin()) {
        LOG_ERROR("SD Card initialization failed");
        // Visual feedback for error
        LEDManager::setLED(0, CRGB::Red);
        LEDManager::showLEDs();
        RoverManager::setEarsDown();  // Reset ears
        delay(1000);
        return;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    LOG_PROD("Rover starting up...");
    
    // Initialize core components first
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    PowerManager::init();
    
    // Initialize LED system first
    LEDManager::init();
    FastLED.clear(true);
    LEDManager::startLoadingAnimation();
    SoundFxManager::startJingle();
    
    // Initialize display
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    PowerManager::setBacklight(255);
    
    // Initialize remaining components
    RoverViewManager::init();
    RoverManager::setEarsDown();  // Ensure ears start down
    SoundFxManager::init();
    
    // Initialize WiFi last
    WiFiManager::setCredentials(primary_ssid, primary_password, backup_ssid, backup_password);
    WiFiManager::init();
    WiFiManager::connectToWiFi();
    
    // Configure GPIO for side button
    pinMode(BOARD_USER_KEY, INPUT_PULLUP);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    
    drawSprite();
    SDManager::init();
}

void syncLEDsForDay() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int currentDay = timeInfo->tm_wday;
    int currentHour = timeInfo->tm_hour % 12;
    if (currentHour == 0) currentHour = 12;
    
    // Define colors for each hour
    static const CRGB hourColors[] = {
        CRGB::Red,           // 1
        CRGB(255, 69, 0),    // 2 (Red/Orange)
        CRGB::Orange,        // 3
        CRGB(255, 165, 0),   // 4 (Orange/Yellow)
        CRGB::Yellow,        // 5
        CRGB::Green,         // 6
        CRGB::Blue,          // 7
        CRGB(75, 0, 130),    // 8 (Blue/Indigo)
        CRGB(75, 0, 130),    // 9 (Indigo)
        CRGB(75, 0, 130),    // 10 (Indigo/Violet)
        CRGB(148, 0, 211),   // 11 (Violet)
        CRGB::Purple         // 12
    };

    // Set the 0th LED to the current hour color at 70% brightness
    LEDManager::setLED(0, hourColors[currentHour - 1]);
    LEDManager::scaleLED(0, 178);  // 70% brightness
    
    // Set LEDs 1-7 based on the current day
    for (int i = 1; i <= 7; i++) {
        LEDManager::setLED(i, CRGB::White);
        if (i <= currentDay) {
            LEDManager::scaleLED(i, 128);  // 50% brightness
        } else {
            LEDManager::scaleLED(i, 28);   // 11% brightness
        }
    }
    
    LEDManager::showLEDs();
}

void readEncoder() {
    encoder.tick();

    // Handle rotation with debouncing
    static unsigned long lastRotaryChange = 0;
    const unsigned long ROTARY_DEBOUNCE = 50;
    
    int newPos = encoder.getPosition();
    static int lastPos = 0;
    
    if (newPos != lastPos && millis() - lastRotaryChange > ROTARY_DEBOUNCE) {
        PowerManager::updateLastActivityTime();
        lastRotaryChange = millis();
        
        // Determine direction and update view
        if (newPos > lastPos) {
            RoverViewManager::nextView();
            SoundFxManager::playRotaryTurnSound(true);
        } else {
            RoverViewManager::previousView();
            SoundFxManager::playRotaryTurnSound(false);
        }
        lastPos = newPos;
        drawSprite();
    }

    // Handle button press with debouncing
    static bool lastButtonState = HIGH;
    static unsigned long lastButtonChange = 0;
    const unsigned long BUTTON_DEBOUNCE = 50;
    
    bool buttonState = digitalRead(ENCODER_KEY);
    
    if (buttonState != lastButtonState && millis() - lastButtonChange > BUTTON_DEBOUNCE) {
        lastButtonChange = millis();
        if (buttonState == LOW) {
            PowerManager::updateLastActivityTime();
            rotaryButtonPressed = true;
            handleRotaryButton();
        }
        lastButtonState = buttonState;
    }
}

void handleRotaryButton() {
    if (rotaryButtonPressed) {
        rotaryButtonPressed = false;
        
        // If we just woke from sleep, first press should show time
        if (PowerManager::getCurrentSleepState() == PowerManager::SleepState::AWAKE && 
            !PowerManager::getShowTime()) {
            PowerManager::setShowTime(true);
        } else {
            // Normal operation - toggle between time and LED modes
            if (!PowerManager::getShowTime()) {
                PowerManager::setShowTime(true);
            } else {
                LEDManager::nextMode();
                SoundFxManager::playRotaryPressSound(static_cast<int>(LEDManager::getMode()));
            }
        }
        drawSprite();
        PowerManager::updateLastActivityTime();
    }
}


void drawSprite() {
    if (!WiFiManager::getTimeInitialized()) {
        RoverViewManager::drawLoadingScreen();
        return;
    }
    spr.fillSprite(TFT_BLACK);
    
    // Adjust x position for better centering
    int x = SCREEN_CENTER_X - 50;  // Adjust based on rover width
    RoverViewManager::drawCurrentView();
    
    spr.pushSprite(0, 0);
}

void handleSideButton() {
    static bool lastState = HIGH;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;
    
    bool currentState = digitalRead(BOARD_USER_KEY);
    
    // Add pull-up resistor
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    
    if (currentState != lastState) {
        if ((millis() - lastDebounceTime) > debounceDelay) {
            lastDebounceTime = millis();
            
            if (currentState == LOW) {  // Button pressed
                RoverManager::setEarsUp();
                SoundFxManager::playSideButtonSound(true);
            } else {  // Button released
                RoverManager::setEarsDown();
                SoundFxManager::playSideButtonSound(false);
            }
            PowerManager::updateLastActivityTime();
            drawSprite();
        }
        lastState = currentState;
    }
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    unsigned long currentMillis = millis();
    
    handleSideButton();
    readEncoder();
    SoundFxManager::updateJingle();
    
    if (PowerManager::getCurrentSleepState() == PowerManager::AWAKE) {
        RoverManager::updateHoverAnimation();
    }
    
    if (currentMillis - lastDisplayUpdate >= 50) {
        PowerManager::checkSleepState();
        switch (PowerManager::getCurrentSleepState()) {
            case PowerManager::AWAKE:
                PowerManager::setBacklight(255);
                LEDManager::updateLEDs();
                drawSprite();
                break;
                
            case PowerManager::DIM_DISPLAY:
                PowerManager::setBacklight(64);
                LEDManager::updateLEDs();
                break;
                
            case PowerManager::DISPLAY_OFF:
                PowerManager::setBacklight(0);
                LEDManager::updateLEDs();
                break;
                
            case PowerManager::DEEP_SLEEP:
                PowerManager::enterDeepSleep();
                break;
        }
        lastDisplayUpdate = currentMillis;
    }
    
    WiFiManager::checkConnection();
}


// Just declare the function (near other function declarations)
void drawBatteryCharging(int x, int y, int size);

void startRecording() {
    if (!SDManager::isInitialized()) return;
    
    File recordFile = SDManager::openFile(RECORD_FILENAME, "w");
    if (!recordFile) {
        LOG_ERROR("Failed to open record file");
        return;
    }
    isRecording = true;
}

void stopRecording() {
    if (!isRecording) return;
    isRecording = false;
    SDManager::closeFile(recordFile);
}


