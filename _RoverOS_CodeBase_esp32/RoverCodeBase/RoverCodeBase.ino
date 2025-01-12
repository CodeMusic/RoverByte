#include <SPI.h>
#include <Wire.h>

// FastLED SPI definitions for ESP32-S3
#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "utilities.h"
#include "Audio.h"
#include <XPowersLib.h>
#include "driver/i2s.h"

#include "PowerManager.h"
#include "RoverViewManager.h"
#include "RoverManager.h"
#include "ColorUtilities.h"
#include "DisplayConfig.h"
#include "SoundFxManager.h"
#include "LEDManager.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

#define CLOCK_PIN 45
CRGB leds[WS2812_NUM_LEDS];

RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);

int value=980;
bool muted=0;
int deb=0;


// Update WiFi credentials at top
const char* primary_ssid = "RevivalNetwork ";
const char* primary_password = "xunjmq84";
const char* backup_ssid = "CodeMusicai";
const char* backup_password = "cnatural";

#define SCREEN_CENTER_X 85  // Adjust this value to shift everything left or right

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


// Add near top with other defines
#define I2S_BCK_PIN  BOARD_VOICE_BCLK   // Pin 46
#define I2S_WS_PIN   BOARD_VOICE_LRCLK  // Pin 40
#define I2S_DOUT_PIN BOARD_VOICE_DIN    // Pin 7

// Single declaration of Audio
Audio audio;
bool isPlayingSound = false;


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
const int WAVE_HEADER_SIZE = 44;
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

// Add these near other global variables
const uint8_t BACKLIGHT_PIN = 38;  // Check your board's backlight pin
const uint8_t PWM_CHANNEL = 0;
const uint8_t PWM_RESOLUTION = 8;
const uint32_t PWM_FREQUENCY = 5000;

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

// Add near other global variables
bool timeInitialized = false;

// Add this new function
void drawLoadingScreen() {
    spr.fillSprite(TFT_BLACK);
    SoundFxManager::playJingle();
    // Draw bone
    int boneX = SCREEN_CENTER_X;
    int boneY = 80;
    int boneWidth = 60;
    int boneHeight = 20;
    int circleRadius = 12;
    
    // Main bone rectangle
    spr.fillRect(boneX - boneWidth/2, boneY - boneHeight/2, 
                 boneWidth, boneHeight, TFT_WHITE);
    
    // Left circles
    spr.fillCircle(boneX - boneWidth/2, boneY - boneHeight/2, 
                   circleRadius, TFT_WHITE);
    spr.fillCircle(boneX - boneWidth/2, boneY + boneHeight/2, 
                   circleRadius, TFT_WHITE);
    
    // Right circles
    spr.fillCircle(boneX + boneWidth/2, boneY - boneHeight/2, 
                   circleRadius, TFT_WHITE);
    spr.fillCircle(boneX + boneWidth/2, boneY + boneHeight/2, 
                   circleRadius, TFT_WHITE);
    
    // Loading text
    spr.setTextFont(4);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("...Loading", SCREEN_CENTER_X, 140);
    
    spr.pushSprite(0, 0);
}

void setupBacklight() {
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(BACKLIGHT_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 255);  // Full brightness
    Serial.println("Backlight setup complete");
}

void setBacklight(uint8_t brightness) {
    bool wasAwake = (currentSleepState == AWAKE);
    isLowBrightness = (brightness < 128);
    
    // If entering dim display state, hide time
    if (wasAwake && currentSleepState == DIM_DISPLAY) {
        showTime = false;
    }
    
    ledcWrite(0, brightness);
    drawSprite();
}

void checkSleepState() {
    unsigned long idleTime = millis() - lastActivityTime;
    SleepState newState = currentSleepState;

    // Add debug prints
    Serial.printf("Idle time: %lu, Current state: %d\n", idleTime, currentSleepState);

    // Determine sleep state based on idle time
    if (idleTime < IDLE_TIMEOUT) {
        newState = AWAKE;
    } else if (idleTime < IDLE_TIMEOUT * 2) {
        newState = DIM_DISPLAY;
    } else if (idleTime < IDLE_TIMEOUT * 3) {
        newState = DISPLAY_OFF;
    } else {
        newState = DEEP_SLEEP;
    }

    // Only update if state has changed
    if (newState != currentSleepState) {
        Serial.printf("State changing from %d to %d\n", currentSleepState, newState);
        
        switch (newState) {
            case AWAKE:
                Serial.println("Going to AWAKE state");
                tft.writecommand(TFT_DISPON);
                setBacklight(255);  // Full brightness
                FastLED.setBrightness(50);
                FastLED.show();
                drawSprite();
                break;
                
            case DIM_DISPLAY:
                Serial.println("Going to DIM_DISPLAY state");
                setBacklight(64);  // 25% brightness
                drawSprite();
                break;

            case DISPLAY_OFF:
                Serial.println("Going to DISPLAY_OFF state");
                tft.writecommand(TFT_DISPOFF);
                setBacklight(0);  // Turn off backlight
                break;

            case DEEP_SLEEP:
                Serial.println("Going to DEEP_SLEEP state");
                tft.writecommand(TFT_DISPOFF);
                setBacklight(0);
                FastLED.setBrightness(0);
                FastLED.show();
                break;
        }
        currentSleepState = newState;
    }
}

void wakeFromSleep() {
    lastActivityTime = millis();
    if (currentSleepState != AWAKE) {
        tft.writecommand(TFT_SLPOUT);  // Wake from sleep
        delay(120);                     // Delay needed after sleep out
        tft.writecommand(TFT_DISPON);  // Turn display on

        ledcWrite(PWM_CHANNEL, 255);  // Full brightness immediately
        lastActivityTime = millis();
        // Restore LED brightness
        FastLED.setBrightness(50);
        FastLED.show();
        
        // Force an immediate display update
        drawSprite();
        currentSleepState = AWAKE;
    }
}

// Add these functions
void init_microphone() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = EXAMPLE_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 8,
        .dma_buf_len = 200,
        .use_apll = 0,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = BOARD_MIC_CLK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = BOARD_MIC_DATA,
    };

    ESP_ERROR_CHECK(i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config));
    ESP_ERROR_CHECK(i2s_set_clk((i2s_port_t)EXAMPLE_I2S_CH, EXAMPLE_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO));
}


void generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate)
{
    // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    uint32_t byte_rate = BYTE_RATE;

    const char set_wav_header[] = {
        'R','I','F','F', // ChunkID
        (char)file_size, (char)(file_size >> 8), (char)(file_size >> 16), (char)(file_size >> 24), // ChunkSize
        'W','A','V','E', // Format
        'f','m','t',' ', // Subchunk1ID
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
        0x01, 0x00, // AudioFormat (1 for PCM)
        0x01, 0x00, // NumChannels (1 channel)
        (char)sample_rate, (char)(sample_rate >> 8), (char)(sample_rate >> 16), (char)(sample_rate >> 24), // SampleRate
        (char)byte_rate, (char)(byte_rate >> 8), (char)(byte_rate >> 16), (char)(byte_rate >> 24), // ByteRate
        0x02, 0x00, // BlockAlign
        0x10, 0x00, // BitsPerSample (16 bits)
        'd','a','t','a', // Subchunk2ID
        (char)wav_size, (char)(wav_size >> 8), (char)(wav_size >> 16), (char)(wav_size >> 24), // Subchunk2Size
    };

    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}

void playErrorSound(int type) {
    switch(type) {
        case 1: // Recording error
            SoundFxManager::playErrorSound(1);
            break;
            
        case 2: // SD card error
            SoundFxManager::playErrorSound(2);
            break;
            
        case 3: // Playback error
            SoundFxManager::playErrorSound(3);
            break;
    }
}

void startRecording() {
    if (isRecording) return;
    
    Serial.println("=== Starting Recording ===");
    
    // Initialize microphone
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = EXAMPLE_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 4,
        .dma_buf_len = 64,
        .use_apll = false,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_PIN_NO_CHANGE,
        .ws_io_num = BOARD_MIC_CLK,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = BOARD_MIC_DATA,
    };

    if (i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL) != ESP_OK) {
        Serial.println("ERROR: Failed to install I2S driver");
        SoundFxManager::playErrorSound(1);
        setEarsDown();
        return;
    }
    
    if (i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config) != ESP_OK) {
        Serial.println("ERROR: Failed to set I2S pins");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(1);
        setEarsDown();
        return;
    }

    // Create new WAV file
    recordFile = SD.open(RECORD_FILENAME, FILE_WRITE);
    if (!recordFile) {
        Serial.println("ERROR: Failed to open file for recording");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        setEarsDown();
        return;
    }

    // Reserve space for the header
    for (int i = 0; i < WAVE_HEADER_SIZE; i++) {
        recordFile.write(0);
    }

    isRecording = true;
    Serial.println("Recording started!");
}

void stopRecording() {
    if (!isRecording) return;

    Serial.println("=== Stopping Recording ===");
    isRecording = false;
    
    // Get final size
    uint32_t file_size = recordFile.size() - WAVE_HEADER_SIZE;
    
    // Generate and write header
    char wav_header[WAVE_HEADER_SIZE];
    generate_wav_header(wav_header, file_size, EXAMPLE_SAMPLE_RATE);
    
    if (!recordFile.seek(0)) {
        Serial.println("ERROR: Failed to seek in file");
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        setEarsDown();
        return;
    }
    
    if (recordFile.write((uint8_t *)wav_header, WAVE_HEADER_SIZE) != WAVE_HEADER_SIZE) {
        Serial.println("ERROR: Failed to write WAV header");
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        setEarsDown();
        return;
    }
    
    recordFile.close();
    i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);

    // Play the recording
    Serial.println("Attempting to play recording...");
    audio.setVolume(21);
    
    if (SD.exists(RECORD_FILENAME)) {
        Serial.println("Found recording file, playing...");
        if (!audio.connecttoFS(SD, RECORD_FILENAME)) {
            Serial.println("ERROR: Failed to start playback");
            SoundFxManager::playErrorSound(3);
            setEarsDown();
            return;
        }
    } else {
        Serial.println("ERROR: Recording file not found!");
        SoundFxManager::playErrorSound(3);
        setEarsDown();
        return;
    }
    
    setEarsDown();  // Put ears down after successful recording/playback start
}


// Add this function near other initialization functions
void initializeBattery() {
    bool result = PPM.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL, BQ25896_SLAVE_ADDRESS);
    if (result) {
        PPM.setSysPowerDownVoltage(3300);
        PPM.setInputCurrentLimit(3250);
        PPM.disableCurrentLimitPin();
        PPM.setChargeTargetVoltage(4208);
        PPM.setPrechargeCurr(64);
        PPM.setChargerConstantCurr(832);
        PPM.enableADCMeasure();
        PPM.enableCharge();
        batteryInitialized = true;
    }
}

// Add this function to calculate battery percentage
int calculateBatteryPercentage(int voltage) {
    // These values might need adjustment based on your battery
    const int maxVoltage = 4200; // 4.2V fully charged
    const int minVoltage = 3300; // 3.3V empty
    
    int percentage = map(voltage, minVoltage, maxVoltage, 0, 100);
    return constrain(percentage, 0, 100);
}

void tryWiFiConnection() {
    if (isRecording) return;  // Skip WiFi connection attempts while recording
    
    if (!isWiFiConnected && 
        (millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL || lastWiFiAttempt == 0)) {
        
        // Try primary network first
        Serial.println("\nAttempting primary WiFi connection...");
        WiFi.begin(primary_ssid, primary_password);
        
        // Wait for primary WiFi with short timeout
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startAttemptTime < 10000) {  // 10 second timeout
            delay(500);
            Serial.print(".");
            leds[0] = CRGB::Blue;
            FastLED.show();
            delay(250);
            leds[0] = CRGB::Black;
            FastLED.show();
            delay(250);
        }
        
        // If primary fails, try backup network
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("\nPrimary WiFi failed, trying backup network...");
            WiFi.disconnect();
            delay(1000);
            WiFi.begin(backup_ssid, backup_password);
            
            // Wait for backup WiFi
            startAttemptTime = millis();
            while (WiFi.status() != WL_CONNECTED && 
                   millis() - startAttemptTime < 10000) {  // 10 second timeout
                delay(500);
                Serial.print(".");
                leds[0] = CRGB::Purple;  // Different color for backup network
                FastLED.show();
                delay(250);
                leds[0] = CRGB::Black;
                FastLED.show();
                delay(250);
            }
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected!");
            Serial.print("Network: ");
            Serial.println(WiFi.SSID());
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            
            // Configure time
            configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
            
            // Wait for time sync
            int attempts = 0;
            while (time(nullptr) < 1000000000 && attempts < 10) {
                delay(500);
                attempts++;
            }
            
            if (time(nullptr) > 1000000000) {
                Serial.println("Time synchronized!");
                isWiFiConnected = true;
            }
        } else {
            Serial.println("\nAll WiFi connections failed!");
            WiFi.disconnect();
        }
        
        lastWiFiAttempt = millis();
    }
}

void connectToWiFi() {
    LOG_DEBUG("Connecting to WiFi...");
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP);  // Looking up while thinking
    
    WiFi.begin(primary_ssid, primary_password);  // Try primary network first
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    // If primary fails, try backup
    if (WiFi.status() != WL_CONNECTED) {
        LOG_DEBUG("Primary WiFi failed, trying backup...");
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(backup_ssid, backup_password);
        attempts = 0;
        
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            attempts++;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        LOG_PROD("WiFi connected!");
        RoverManager::setTemporaryExpression(RoverManager::BIG_SMILE, 1000);  // Success smile
    } else {
        LOG_PROD("WiFi connection failed!");
        RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 1000);  // Error expression
    }
}

// Generic error handler
void handleError(const char* errorMessage) {
    LOG_PROD("Error: %s", errorMessage);
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 1000);
}

// For radio button release
void handleRadioButtonRelease() {
    // ... existing radio button code ...
    
    delay(1000);  // Wait a second
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP, 1000);
}

// Example usage in various scenarios
void processAPIRequest(bool success = false) {  // Parameter with default value
    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP);  // Looking up while thinking
    
    if (success) {
        RoverManager::setTemporaryExpression(RoverManager::BIG_SMILE, 1000);
    } else {
        handleError("API request failed");
    }
}

void handleSDCardOperation() {
    if (!SD.begin()) {
        handleError("SD Card initialization failed");
        return;
    }
    // ... rest of SD card code ...
}

void setup() {
    Serial.begin(115200);
    
    // Set logging level (can be changed via commands later)
    #ifdef DEBUG_MODE
        CURRENT_LOG_LEVEL = LOG_DEBUG;
    #endif
    
    LOG_PROD("Rover starting up...");
    
    // Initialize FastLED first for status indicators
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    
    // Initialize side button first
    pinMode(BOARD_USER_KEY, INPUT);
    delay(100); // Give time for pin to stabilize
    
    // Initialize encoder (no begin needed for this library)
    // encoder is already initialized by its constructor
    
    // Initialize audio
    pinMode(BOARD_VOICE_DIN, OUTPUT);
    
    // Rest of setup
    pinMode(46, OUTPUT);
    digitalWrite(46, HIGH);
    
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    pinMode(0, INPUT_PULLUP);
    
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);

    // Initialize audio
    audio.setPinout(BOARD_VOICE_BCLK, BOARD_VOICE_LRCLK, BOARD_VOICE_DIN);
    audio.setVolume(21); // 0...21
    Serial.println("Audio initialized");
    
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    initializeBattery();
    

    spr.createSprite(tft.width(), tft.height());
    spr.setTextDatum(4);
    spr.setSwapBytes(true);
    spr.setFreeFont(&Orbitron_Light_24);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);

    leds[0] = CRGB::Red;
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    leds[3] = CRGB::White;
    leds[4] = CRGB::Red;
    leds[5] = CRGB::White;
    leds[6] = CRGB::White;
    leds[7] = CRGB::White;
    FastLED.setBrightness(50);
    FastLED.show();
    drawSprite();

    // Draw loading screen and start LED animation
    drawLoadingScreen();
    LEDManager::startLoadingAnimation();
    
    // Connect to WiFi with visual feedback and retries
    Serial.println("Connecting to WiFi...");
    WiFi.begin(primary_ssid, primary_password);  // Try primary network first
    
    // Wait for WiFi with timeout and visual feedback
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < 20000) {  // 20 second timeout
        
        delay(500);
        Serial.print(".");
        // Flash first LED to show we're trying to connect
        leds[0] = CRGB::Blue;
        FastLED.show();
        delay(250);
        leds[0] = CRGB::Black;
        FastLED.show();
        delay(250);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 500);
        Serial.println("\nPrimary WiFi failed, trying backup...");
        WiFi.disconnect();
        delay(1000);
        WiFi.begin(backup_ssid, backup_password);  // Try backup network
        
        startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && 
               millis() - startAttemptTime < 20000) {
            delay(500);
            Serial.print(".");
            leds[0] = CRGB::Purple;
            FastLED.show();
            delay(250);
            leds[0] = CRGB::Black;
            FastLED.show();
            delay(250);
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        RoverManager::setTemporaryExpression(RoverManager::BIG_SMILE, 500);
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Configure time
        configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        
        // Wait for time sync with visual feedback
        Serial.println("Waiting for time sync...");
        int attempts = 0;
        while (time(nullptr) < 1000000000 && attempts < 30) {  // 30 attempts max
            delay(500);
            Serial.print(".");
            // Flash first two LEDs to show we're syncing time
            leds[0] = CRGB::Green;
            leds[1] = CRGB::Green;
            FastLED.show();
            delay(250);
            leds[0] = CRGB::Black;
            leds[1] = CRGB::Black;
            FastLED.show();
            delay(250);
            attempts++;
            drawLoadingScreen();  // Update loading screen while waiting
            
        }
        
        if (time(nullptr) > 1000000000) {
            Serial.println("\nTime synchronized!");
            timeInitialized = true;  // Set initialized flag
            time_t now = time(nullptr);
            struct tm* timeInfo = localtime(&now);
            Serial.printf("Current time: %02d:%02d:%02d\n", 
                timeInfo->tm_hour, 
                timeInfo->tm_min, 
                timeInfo->tm_sec);
        } else {
            Serial.println("\nTime sync failed!");
        }
    } else {
        Serial.println("\nWiFi connection failed! Setting default time...");
        RoverManager::setTemporaryExpression(RoverManager::LOOKING_DOWN, 500);
        // Set default time to December 31st, 5:45 PM
        struct tm timeinfo = { 0 };
        timeinfo.tm_year = 2023 - 1900;  // Years since 1900
        timeinfo.tm_mon = 11;            // 0-11 for month (11 = December)
        timeinfo.tm_mday = 31;           // 31st
        timeinfo.tm_hour = 17;           // 5 PM (17:00)
        timeinfo.tm_min = 45;            // 45 minutes
        timeinfo.tm_sec = 0;             // 0 seconds
        timeinfo.tm_isdst = 0;           // No daylight saving
        
        timeval tv = { mktime(&timeinfo), 0 };
        settimeofday(&tv, NULL);
        
        Serial.println("Default time set!");
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        Serial.printf("Current time set to: %02d:%02d:%02d\n", 
            timeInfo->tm_hour, 
            timeInfo->tm_min, 
            timeInfo->tm_sec);
    }
    
    // Initialize LEDs
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    LEDManager::updateLEDs();  // Initial LED update

    setupBacklight();
    
    // Initialize buttons with internal pull-ups
    pinMode(BOARD_USER_KEY, INPUT_PULLUP);
    pinMode(ENCODER_KEY, INPUT_PULLUP);
    delay(100);  // Give time for pins to stabilize
    
    // Configure both buttons as wake sources with pull-ups enabled
    const uint64_t ext_wakeup_pin_mask = 
        (1ULL << BOARD_USER_KEY) | 
        (1ULL << ENCODER_KEY);
    esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_mask, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    PowerManager::init();
    RoverViewManager::init();

    // Initialize display with time hidden and large rover
    showTime = false;
    drawSprite();

    // Initialize LED manager
    LEDManager::init();

    // After WiFi and time are configured
    if (isWiFiConnected) {
        LEDManager::stopLoadingAnimation();  // This will switch to FULL_MODE
    } else {
        // If WiFi failed, still stop animation but maybe show an error state
        LEDManager::stopLoadingAnimation();
    }

    // Near the beginning of setup()
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 || wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        PowerManager::wakeFromSleep();
    }
}

void syncLEDsForDay() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int currentDay = timeInfo->tm_wday;  // 0 = Sunday, 6 = Saturday
    int currentHour = timeInfo->tm_hour % 12;  // Convert to 12-hour format

    // Define colors for each hour
    CRGB hourColors[] = {
        CRGB::Red,           // 1
        CRGB(255, 69, 0),    // 2 (Red/Orange)
        CRGB::Orange,        // 3
        CRGB(255, 165, 0),   // 4 (Orange/Yellow)
        CRGB::Yellow,        // 5
        CRGB::Green,         // 6
        CRGB(0, 128, 128),   // 7 (Green/Blue)
        CRGB::Blue,          // 8
        CRGB(75, 0, 130),    // 9 (Blue/Indigo)
        CRGB(75, 0, 130),    // 10 (Indigo)
        CRGB(75, 0, 130),    // 11 (Indigo/Violet)
        CRGB(148, 0, 211)    // 12 (Violet)
    };

    // Set the 0th LED to the current hour color at 70% brightness
    leds[0] = hourColors[currentHour - 1];
    leds[0].nscale8(178);  // Scale to 70% brightness

    // Set LEDs 1-7 based on the current day
    for (int i = 1; i <= 7; i++) {
        if (i <= currentDay) {
            // Turn on LEDs for the current day and the rest of the week at 50% brightness
            leds[i] = CRGB::White;
            leds[i].nscale8(128);  // Scale to 50% brightness
        } else {
            // Turn off past days at 11% brightness
            leds[i] = CRGB::White;
            leds[i].nscale8(28);  // Scale to 11% brightness
        }
    }

    FastLED.show();
}

void readEncoder() {
    encoder.tick();

    // Handle rotation
    int newPos = encoder.getPosition();
    static int lastPos = 0;
    
    if (newPos != lastPos) {
        PowerManager::wakeFromSleep();  // Wake on any rotation
        PowerManager::updateLastActivityTime();
        
        if (newPos > lastPos) {
            RoverViewManager::nextView();
            SoundFxManager::playRotaryTurnSound(true);
        } else {
            RoverViewManager::previousView();
            SoundFxManager::playRotaryTurnSound(false);
        }
        lastPos = newPos;
    }

    // Handle button press
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(ENCODER_KEY);
    
    if (buttonState != lastButtonState) {
        if (buttonState == LOW) {  // Button pressed

            if (PowerManager::getCurrentSleepState() == PowerManager::AWAKE) { 
                if (showTime) {  // If currently showing small rover with time
                    LEDManager::nextMode();  // Start LED mode cycling
                    SoundFxManager::playRotaryPressSound(static_cast<int>(LEDManager::getMode()));
                } else {  // If in LED mode
                    showTime = true;
                }
            } else {            
                PowerManager::wakeFromSleep();  // Wake on button press
            }
           PowerManager::updateLastActivityTime();
            drawSprite();
        }
        lastButtonState = buttonState;
    }

    handleRotaryButton();
}

void handleRotaryButton() {
    if (rotaryButtonPressed) {
        rotaryButtonPressed = false;
        
        // Toggle between showing time and LED modes
        if (!showTime) {
            showTime = true;
        } else {
            LEDManager::nextMode();
            SoundFxManager::playRotaryPressSound(static_cast<int>(LEDManager::getMode()));
        }
        drawSprite();
        
        // Reset sleep timer on button press
        PowerManager::updateLastActivityTime();
    }
}


void drawSprite() {
    if (!timeInitialized) {
        drawLoadingScreen();
        return;
    }
    spr.fillSprite(TFT_BLACK);
    
    // Draw Rover in top section
    RoverManager::drawRover(
        RoverManager::getCurrentMood(),
        RoverManager::earsPerked,
        !showTime,
        tft.width() / 2,
        showTime ? 50 : 80
    );
    
    // Draw status bar and current view
    RoverViewManager::drawStatusBar();
    RoverViewManager::drawCurrentView();
    
    spr.pushSprite(0, 0);
}

void handleSideButton() {
    static bool lastState = HIGH;
    static unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
    bool currentState = digitalRead(BOARD_USER_KEY);
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentState != lastState) {

            if (PowerManager::getCurrentSleepState() != PowerManager::AWAKE)
            {
                PowerManager::wakeFromSleep();  // Wake on side button
            }
            PowerManager::updateLastActivityTime();
            lastDebounceTime = millis();
            lastState = currentState;
            
            if (currentState == LOW) {
                if (!RoverManager::earsPerked) {
                    setEarsUp();
                    RoverManager::setTemporaryExpression(RoverManager::HAPPY);
                    drawSprite();
                    SoundFxManager::playSideButtonSound(true);
                }
            } else {
                if (RoverManager::earsPerked) {
                    setEarsDown();
                    RoverManager::setTemporaryExpression(RoverManager::LOOKING_UP, 1000);
                    drawSprite();
                    SoundFxManager::playSideButtonSound(false);
                }
            }
        }
    }
}

void loop() {
    handleSideButton();
    readEncoder();

    static unsigned long lastDisplayUpdate = 0;
    unsigned long currentMillis = millis();

    if (PowerManager::getCurrentSleepState() == PowerManager::AWAKE) {
        RoverManager::updateHoverAnimation();
        drawSprite();
    }
    
    if (currentMillis - lastDisplayUpdate >= 50) {
        PowerManager::checkSleepState();
        
        switch (PowerManager::getCurrentSleepState()) {
            case PowerManager::AWAKE:
                setBacklight(255);
                drawSprite();
                LEDManager::updateLEDs();
                break;
                
            case PowerManager::DIM_DISPLAY:
                setBacklight(64);
                drawSprite();
                LEDManager::updateLEDs();
                break;
                
            case PowerManager::DISPLAY_OFF:
                tft.writecommand(TFT_DISPOFF);
                setBacklight(0);
                LEDManager::updateLEDs();
                break;
                
            case PowerManager::DEEP_SLEEP:
                PowerManager::enterDeepSleep();
                break;
        }
        
        lastDisplayUpdate = currentMillis;
    }
    
    if (isPlayingSound) {
        audio.loop();
    }
    
    tryWiFiConnection();

    if (currentMillis - lastExpressionChange >= nextExpressionInterval) {
        RoverManager::setRandomMood();  // Assuming this exists, if not we'll need to create it
        lastExpressionChange = currentMillis;
        // Set next interval between 1-10 minutes (60000-600000 ms)
        nextExpressionInterval = random(60000, 600000);
    }

    readEncoder();  // At end of loop too
}

// Add audio callback
void audio_eof_mp3(const char *info) {
    Serial.printf("Audio playback finished: %s\n", info);
    // Delete temporary recording after playback
    if (!SD.remove(RECORD_FILENAME)) {
        Serial.println("Failed to delete temporary recording file");
        playErrorSound(2);
    }
    isPlayingSound = false;
}

// Make sure setEarsUp and setEarsDown are defined only once
void setEarsUp() {
    RoverManager::earsPerked = true;
    drawSprite();
    FastLED.show();
}

void setEarsDown() {
    RoverManager::earsPerked = false;
    drawSprite();
    FastLED.show();
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

// Just declare the function (near other function declarations)
void drawBatteryCharging(int x, int y, int size);
