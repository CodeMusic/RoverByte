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
#include "src/PsychicCortex/WiFiManager.h"
#include "src/PrefrontalCortex/SDManager.h"
#include "src/PsychicCortex/NFCManager.h"
#include "src/SomatosensoryCortex/UIManager.h"

// Core configuration
#define SCREEN_CENTER_X 85
#define CLOCK_PIN 45

// Global objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

// Global state
bool isLowBrightness = false;
bool showTime = false;
static int pos = 0;
int value = 980;
bool muted = 0;
int deb = 0;


// Function declarations
void drawBatteryCharging(int x, int y, int size);

// Audio instance
Audio audio;
bool isPlayingSound = false;

// Add after other global variables (around line 50)
bool rotaryButtonPressed = false;
bool isRecording = false;
File recordFile;
const char* RECORD_FILENAME = "/recording.wav";
bool isInitialized = false;

void setup() {
    Serial.begin(115200);
    isInitialized = false;
    // Initialize display first
    tft.init();
    tft.setRotation(0);
    tft.writecommand(TFT_SLPOUT);
    delay(120);  // This delay is necessary for display
    tft.writecommand(TFT_DISPON);
    spr.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
    spr.setTextDatum(MC_DATUM);
    
    // Show initial loading screen
    RoverViewManager::drawLoadingScreen("Contacting the RoverVerse...");
    spr.pushSprite(0, 0);  // Important: Push the sprite!
    
    // Initialize core hardware (non-blocking)
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    Wire.setClock(100000);
    
    // Configure SPI pins
    pinMode(TFT_CS, OUTPUT);
    pinMode(BOARD_SD_CS, OUTPUT);
    pinMode(BOARD_LORA_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(BOARD_SD_CS, HIGH);
    digitalWrite(BOARD_LORA_CS, HIGH);
    
    // Initialize managers in order
    SoundFxManager::init();
    PowerManager::init();
    LEDManager::init();
    RoverViewManager::init();
    SDManager::init();
    
    // Start background processes
    NFCManager::startBackgroundInit();
    WiFiManager::init();
}

void drawSprite() {
    spr.fillSprite(TFT_BLACK);
    
    if (!WiFiManager::getTimeInitialized()) {
        RoverViewManager::drawLoadingScreen("Locating temporal coordinates...");
        spr.pushSprite(0, 0);
        return;
    }
    
    if (RoverViewManager::hasActiveNotification()) {
        RoverViewManager::drawNotification();
        spr.pushSprite(0, 0);
        return;
    }
    
    RoverViewManager::drawCurrentView();
    spr.pushSprite(0, 0);
}

void loop() {
    static unsigned long lastDraw = 0;
    const unsigned long DRAW_INTERVAL = 50;  // 20fps is enough for loading
    static bool soundStarted = false;
    if (!isInitialized) {
        RoverViewManager::drawLoadingScreen("");
    }

    // Always handle basic updates
    WiFiManager::checkConnection();
    LEDManager::updateLoadingAnimation();
     // Play startup sound once
    if (!soundStarted) {
        SoundFxManager::playStartupSound();
        soundStarted = true;
    }  

    if (millis() - lastDraw >= DRAW_INTERVAL) {

        
        // Always show loading until fully initialized
        if (!WiFiManager::getTimeInitialized()) {
            const char* loadingStatus;
            if (!WiFiManager::isConnected()) {
                loadingStatus = "Connecting to WiFi...";
            } else {
                loadingStatus = "Synchronizing time...";
                WiFiManager::syncTime();
            }
            RoverViewManager::drawLoadingScreen(loadingStatus);
            
        } else {
            // Only show main view after full initialization
            if (RoverViewManager::hasActiveNotification()) {
                RoverViewManager::drawNotification();
            } else {
                RoverViewManager::drawCurrentView();
            }
        }
        
        spr.pushSprite(0, 0);
        lastDraw = millis();
    }
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


