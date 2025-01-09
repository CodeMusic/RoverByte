#include <SPI.h>
#include <Wire.h>

// FastLED SPI definitions for ESP32-S3
#define FASTLED_ESP32_SPI_BUS FSPI  // Use FSPI instead of VSPI for ESP32-S3
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
#include "RoverManager.h"
#include "PowerManager.h"
#include "RoverViewManager.h"
#include "ColorUtilities.h"
#include "DisplayConfig.h"
// Add FastLED SPI definitions
#define FASTLED_ALL_PINS_HARDWARE_SPI
#define FASTLED_ESP32_SPI_BUS VSPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 16

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

bool isWeekMode = false;  // false = full mode, true = week mode
const uint8_t PAST_BRIGHTNESS = 0;      // 0%
const uint8_t FUTURE_BRIGHTNESS = 178;  // 70%
const uint8_t TODAY_BRIGHTNESS = 232;   // 91%
const uint8_t MONTH_DIM = 64;          // 25%

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
        // Immediately turn on display and backlight
        tft.writecommand(TFT_DISPON);
        ledcWrite(PWM_CHANNEL, 255);  // Full brightness immediately
        
        // Reset sleep state
        currentSleepState = AWAKE;
        
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
            playTone(1000, 200);
            delay(100);
            playTone(800, 200);
            delay(100);
            playTone(600, 400);
            break;
            
        case 2: // SD card error
            playTone(800, 200);
            delay(100);
            playTone(800, 200);
            delay(100);
            playTone(400, 400);
            break;
            
        case 3: // Playback error
            playTone(600, 200);
            delay(100);
            playTone(600, 200);
            delay(100);
            playTone(300, 400);
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
        playErrorSound(1);
        setEarsDown();
        return;
    }
    
    if (i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config) != ESP_OK) {
        Serial.println("ERROR: Failed to set I2S pins");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        playErrorSound(1);
        setEarsDown();
        return;
    }

    // Create new WAV file
    recordFile = SD.open(RECORD_FILENAME, FILE_WRITE);
    if (!recordFile) {
        Serial.println("ERROR: Failed to open file for recording");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        playErrorSound(2);
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
        playErrorSound(2);
        setEarsDown();
        return;
    }
    
    if (recordFile.write((uint8_t *)wav_header, WAVE_HEADER_SIZE) != WAVE_HEADER_SIZE) {
        Serial.println("ERROR: Failed to write WAV header");
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        playErrorSound(2);
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
            playErrorSound(3);
            setEarsDown();
            return;
        }
    } else {
        Serial.println("ERROR: Recording file not found!");
        playErrorSound(3);
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
    leds[2] = CRGB::Red;
    leds[3] = CRGB::Green;
    leds[4] = CRGB::Red;
    leds[5] = CRGB::Blue;
    leds[6] = CRGB::Red;
    leds[7] = CRGB::Red;
    FastLED.setBrightness(50);
    FastLED.show();
    drawSprite();

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
        }
        
        if (time(nullptr) > 1000000000) {
            Serial.println("\nTime synchronized!");
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
    updateLEDs();  // Initial LED update

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
        if (newPos > lastPos) {
            RoverViewManager::nextView();
            playTone(1047, 20);
        } else {
            RoverViewManager::previousView();
            playTone(1568, 20);
        }
        lastPos = newPos;
    }

    // Handle button press with proper mode switching
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(ENCODER_KEY);
    
    if (buttonState != lastButtonState) {
        delay(50); // Simple debounce
        if (buttonState == LOW) {
            isWeekMode = !isWeekMode;  // Toggle mode
            showTime = true;           // Show time when mode changes
            
            // Force immediate LED updates
            FastLED.clear();  // Clear existing LED states
            updateLEDs();     // Update with new mode
            FastLED.show();   // Make sure changes are displayed
            drawSprite();
            
            // Play different tones for different modes
            if (isWeekMode) {
                playTone(1000, 100);  // Higher tone for week mode
                Serial.println("Switched to Week Mode");
            } else {
                playTone(800, 100);   // Lower tone for full mode
                Serial.println("Switched to Full Mode");
            }
        }
        lastButtonState = buttonState;
    }
}


void drawSprite() {
    spr.fillSprite(TFT_BLACK);
    
    // Draw Rover in top section
    RoverManager::drawRover(
        RoverManager::getCurrentMood(),
        RoverManager::earsPerked,
        !showTime,  // Reversed logic: large when NOT showing time
        tft.width() / 2,
        showTime ? 50 : 80
    );
    
    // Draw status bar between rover and view
    drawStatusBar();
    
    // Draw current view in bottom section
    RoverViewManager::drawCurrentView();
    
    spr.pushSprite(0, 0);
}

void drawStatusBar() {
    // Get current time
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Status bar positioning - move it up slightly
    int statusBarY = 170;  // Adjusted from 180 to 170
    
    // Left side: Month color square with day number
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    
    int dateWidth = 40;
    int dateHeight = 30;
    int dateX = 2;  // Keep at far left edge
    
    // Draw month color square
    uint32_t monthTftColor;  // Store the color for text background
    if (monthColor1.r == monthColor2.r && 
        monthColor1.g == monthColor2.g && 
        monthColor1.b == monthColor2.b) {
        monthTftColor = spr.color565(monthColor1.r, monthColor1.g, monthColor1.b);
        spr.fillRect(dateX, statusBarY, dateWidth, dateHeight, monthTftColor);
    } else {
        // For gradient, use first color for text background
        monthTftColor = spr.color565(monthColor1.r, monthColor1.g, monthColor1.b);
        for (int i = 0; i < dateWidth; i++) {
            float ratio = (float)i / dateWidth;
            uint8_t r = monthColor1.r + (monthColor2.r - monthColor1.r) * ratio;
            uint8_t g = monthColor1.g + (monthColor2.g - monthColor1.g) * ratio;
            uint8_t b = monthColor1.b + (monthColor2.b - monthColor1.b) * ratio;
            uint32_t tftColor = spr.color565(r, g, b);
            spr.drawFastVLine(dateX + i, statusBarY, dateHeight, tftColor);
        }
    }
    
    // Draw day number with month color as background
    char dayStr[3];
    sprintf(dayStr, "%d", timeInfo->tm_mday);
    spr.setTextFont(2);
    spr.setTextColor(TFT_WHITE, monthTftColor);
    spr.drawString(dayStr, dateX + dateWidth/2, statusBarY + dateHeight/2);
    
    // Center/Right side: Rotating status display
    int statusX = 120;  // Center position
    
    // Alternate between different status displays
    static int statusRotation = 0;
    if (millis() - lastStatusUpdate >= STATUS_CHANGE_INTERVAL) {
        statusRotation = (statusRotation + 1) % 3;
        lastStatusUpdate = millis();
    }

    spr.setTextFont(2);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    
    switch (statusRotation) {
        case 0:
            spr.drawString("Level: 21", statusX, statusBarY + dateHeight/2);
            break;
        case 1:
            spr.drawString("XP: 1337/2000", statusX, statusBarY + dateHeight/2);
            break;
        case 2:
            // Battery status with white outline and yellow charging bolt
            if (PowerManager::isCharging()) {
                int batteryWidth = 25;
                int batteryHeight = 12;
                int batteryX = statusX - batteryWidth/2;
                int batteryY = statusBarY + dateHeight/2 - batteryHeight/2;
                
                // Draw white outline for main battery body
                spr.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
                
                // Draw white battery tip
                spr.fillRect(batteryX + batteryWidth, batteryY + 3, 2, 6, TFT_WHITE);
                
                // Draw yellow lightning bolt
                spr.fillTriangle(
                    batteryX + 10, batteryY + 2,      // Top point
                    batteryX + 15, batteryY + 6,      // Middle right
                    batteryX + 12, batteryY + 6,      // Middle left
                    TFT_YELLOW
                );
                spr.fillTriangle(
                    batteryX + 12, batteryY + 6,      // Middle top
                    batteryX + 15, batteryY + 6,      // Middle right
                    batteryX + 10, batteryY + 10,     // Bottom point
                    TFT_YELLOW
                );
                
                // Draw battery percentage text
                char batteryStr[5];
                sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                spr.drawString(batteryStr, statusX + 20, statusBarY + dateHeight/2);
            } else {
                // Draw battery icon with white outline
                int batteryWidth = 25;
                int batteryHeight = 12;
                int batteryX = statusX - batteryWidth/2;
                int batteryY = statusBarY + dateHeight/2 - batteryHeight/2;
                
                // Draw white outline for main battery body
                spr.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
                
                // Draw white battery tip
                spr.fillRect(batteryX + batteryWidth, batteryY + 3, 2, 6, TFT_WHITE);
                
                // Fill battery based on percentage
                int fillWidth = (batteryWidth - 4) * PowerManager::getBatteryPercentage() / 100;
                spr.fillRect(batteryX + 2, batteryY + 2, fillWidth, batteryHeight - 4, TFT_WHITE);
                
                // Draw battery percentage text
                char batteryStr[5];
                sprintf(batteryStr, "%d%%", PowerManager::getBatteryPercentage());
                spr.drawString(batteryStr, statusX + 20, statusBarY + dateHeight/2);
            }
            break;
    }

    // Draw todo list section with word wrapping
    spr.fillRect(2, 195, 280, 120, 0xC618);
    spr.drawRect(4, 193, 284, 124, TFT_DARKGREY);
    
    // Draw title
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, 0xC618);
    
    // For virtues and chakras, implement word wrapping
    const int maxWidth = 260;  // Maximum width for text
    const int lineHeight = 20; // Height between lines
    int currentY = 225;        // Starting Y position for text
    
    spr.setTextFont(2);
    
    // Replace getCurrentViewText() with actual text based on current view
    String text;
    int currentView = RoverViewManager::getCurrentView();
    
    if (currentView == 1) {  // Virtues view
        text = "Virtues: Wisdom, Justice, Courage, Temperance, Faith, Hope, and Love. These guide our actions and shape our character.";
    } else if (currentView == 2) {  // Chakras view
        text = "Chakras: Root, Sacral, Solar Plexus, Heart, Throat, Third Eye, and Crown. Energy centers for balance and healing.";
    } else {  // Default view
        text = "Today's Tasks: 1. Update Code 2. Test Features 3. Fix Bugs";
    }
    
    // Word wrap implementation
    String line = "";
    String currentWord = "";
    int lineWidth = 0;
    
    for (int i = 0; i < text.length(); i++) {
        if (text[i] == ' ' || i == text.length() - 1) {
            // Add last character if at end of text
            if (i == text.length() - 1) {
                currentWord += text[i];
            }
            
            // Check if adding this word would exceed width
            int wordWidth = spr.textWidth(currentWord.c_str());
            if (lineWidth + wordWidth > maxWidth) {
                // Draw current line and start new one
                spr.drawString(line, SCREEN_CENTER_X, currentY);
                currentY += lineHeight;
                line = currentWord;
                lineWidth = wordWidth;
            } else {
                // Add word to current line
                if (line.length() > 0) {
                    line += " ";
                    lineWidth += spr.textWidth(" ");
                }
                line += currentWord;
                lineWidth += wordWidth;
            }
            currentWord = "";
        } else {
            currentWord += text[i];
        }
    }
    
    // Draw any remaining text
    if (line.length() > 0) {
        spr.drawString(line, SCREEN_CENTER_X, currentY);
    }
}

void handleSideButton() {
    static bool lastState = HIGH;
    static unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
    bool currentState = digitalRead(BOARD_USER_KEY);
    
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentState != lastState) {
            wakeFromSleep();
            lastDebounceTime = millis();
            lastState = currentState;
            
            if (currentState == LOW) {
                if (!RoverManager::earsPerked) {
                    setEarsUp();
                    drawSprite();

                    // Radio-style chirp sequence
                    playTone(2500, 30);  // High chirp
                    delay(20);
                    playTone(1800, 40);  // Medium chirp
                    delay(20);
                    playTone(2200, 35);  // Response chirp
                    delay(15);
                    playTone(2600, 25);  // Final blip
                }
            } else {
                if (RoverManager::earsPerked) {
                    setEarsDown();
                    drawSprite();
                    
                    // Radio sign-off chirp
                    playTone(2200, 35);
                    delay(20);
                    playTone(1800, 45);  // Lower tone for "down"
                }
            }
        }
    }
}

void playTone(int frequency, int duration) {
    isPlayingSound = true;

    // Set LED color based on frequency
    leds[0] = ColorUtilities::getColorForFrequency(frequency);
    FastLED.show();

    // Configure PWM for tone
    ledcSetup(TONE_PWM_CHANNEL, frequency, 8);
    ledcAttachPin(TONE_PIN, TONE_PWM_CHANNEL);
    ledcWrite(TONE_PWM_CHANNEL, 127);

    delay(duration);

    ledcWrite(TONE_PWM_CHANNEL, 0);
    ledcDetachPin(TONE_PIN);

    isPlayingSound = false;
}

void updateLEDs() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Create a 3-state blink cycle (0, 1, 2) using integer division of seconds
    int blinkState = (timeInfo->tm_sec % 3);
    
    if (isWeekMode) {
        // Week Mode
        // LED 0: Month color - blink between colors or solid
        CRGB monthColor1, monthColor2;
        ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
        
        if (monthColor1 == monthColor2) {
            // Solid color month - blink between color and off
            if (timeInfo->tm_sec % 2 == 0) {
                monthColor1.nscale8(184);  // 72% brightness
                leds[0] = monthColor1;
            } else {
                leds[0] = CRGB::Black;
            }
        } else {
            // Two-color month - cycle between colors and off
            switch(blinkState) {
                case 0: 
                    monthColor1.nscale8(184);  // 72% brightness
                    leds[0] = monthColor1; 
                    break;
                case 1: 
                    monthColor2.nscale8(184);  // 72% brightness
                    leds[0] = monthColor2; 
                    break;
                case 2: 
                    leds[0] = CRGB::Black; 
                    break;
            }
        }
        
        // LEDs 1-7: Days of week (Sunday=1 to Saturday=7)
        CRGB dayColors[] = {
            CRGB::Red,          // Sunday
            CRGB::Orange,       // Monday
            CRGB::Yellow,       // Tuesday
            CRGB::Green,        // Wednesday
            CRGB::Blue,         // Thursday
            CRGB(75, 0, 130),   // Friday (Indigo)
            CRGB(148, 0, 211)   // Saturday (Violet)
        };
        
        for (int i = 1; i <= 7; i++) {
            if (i - 1 < timeInfo->tm_wday) {
                // Past days are off
                leds[i] = CRGB::Black;
            } 
            else if (i - 1 == timeInfo->tm_wday) {
                // Current day blinks at 72% brightness
                if (timeInfo->tm_sec % 2 == 0) {
                    CRGB dayColor = dayColors[i - 1];
                    dayColor.nscale8(184);  // 72% brightness
                    leds[i] = dayColor;
                } else {
                    leds[i] = CRGB::Black;
                }
            }
            else {
                // Future days at 30% brightness
                CRGB dayColor = dayColors[i - 1];
                dayColor.nscale8(77);  // 30% brightness
                leds[i] = dayColor;
            }
        }
    } else {
        // LED 0: Day of week color
        leds[0] = ColorUtilities::getDayColor(timeInfo->tm_wday + 1);
        
        // LED 1: Week number of month (base 8)
        int weekOfMonth = (timeInfo->tm_mday - 1) / 7;  // 0-3 or 4
        switch(weekOfMonth) {
            case 0: leds[1] = CRGB::Red; break;
            case 1: leds[1] = CRGB(255, 100, 0); break;  // Orange
            case 2: leds[1] = CRGB::Yellow; break;
            case 3: leds[1] = CRGB::Green; break;
            default: leds[1] = CRGB::Blue; break;  // 5th week if exists
        }
        
        // LED 2: Month (base 12)
        CRGB monthColor1, monthColor2;
        ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
        if (monthColor1 == monthColor2) {
            leds[2] = monthColor1;  // Solid color if both are the same
        } else {
            switch(blinkState) {
                case 0: leds[2] = monthColor1; break;
                case 1: leds[2] = monthColor2; break;
                case 2: leds[2] = CRGB::Black; break;
            }
        }
        
        // LED 3: Hours (base 12)
        int hour12 = timeInfo->tm_hour % 12;
        if (hour12 == 0) hour12 = 12;
        CRGB hourColor1, hourColor2;
        ColorUtilities::getMonthColors(hour12, hourColor1, hourColor2);
        if (hourColor1 == hourColor2) {
            leds[3] = hourColor1;  // Solid color if both are the same
        } else {
            switch(blinkState) {
                case 0: leds[3] = hourColor1; break;
                case 1: leds[3] = hourColor2; break;
                case 2: leds[3] = CRGB::Black; break;
            }
        }
        
        // LED 4-5: Minutes (base 8)
        int minutes = timeInfo->tm_min;
        int minTens = minutes / 8;
        int minOnes = minutes % 8;
        leds[4] = ColorUtilities::getBase8Color(minTens);
        leds[5] = ColorUtilities::getBase8Color(minOnes);
        
        // LED 6-7: Day of month (base 8)
        int day = timeInfo->tm_mday;  // 8
        int dayTens = day / 8;        // 8/8 = 1 (red)
        int dayOnes = day % 8;        // 8%8 = 0 (off)
        
        // Base-8 colors:
        // 0 = off/black
        // 1 = red
        // 2 = orange
        // 3 = yellow
        // 4 = green
        // 5 = blue
        // 6 = indigo
        // 7 = violet
        
        leds[6] = ColorUtilities::getBase8Color(dayOnes);  // Should be off (0)
        leds[7] = ColorUtilities::getBase8Color(dayTens);  // Should be red (1)
    }
    
    FastLED.show();
}

void enterSleepMode() {
    Serial.println("Entering sleep mode...");
    isInSleepMode = true;
    
    // Turn off screen
    spr.fillSprite(TFT_BLACK);
    spr.pushSprite(0, 0);
    tft.writecommand(TFT_DISPOFF);  // Turn off display
    
    // Turn off all LEDs
    fill_solid(leds, WS2812_NUM_LEDS, CRGB::Black);
    FastLED.show();
    
    // Reset button states
    rotaryButtonPressed = false;
    sideButtonPressed = false;
}

void exitSleepMode() {
    Serial.println("Exiting sleep mode...");
    isInSleepMode = false;
    
    // Turn on screen
    tft.writecommand(TFT_DISPON);  // Turn on display
    drawSprite();  // Redraw the display
    
    // Restore LED state
    updateLEDs();
    
    // Reset button states
    rotaryButtonPressed = false;
    sideButtonPressed = false;
}

void loop() {
    // Call encoder reading more frequently
    readEncoder();  // At start of loop

    handleSideButton();

    static unsigned long lastDisplayUpdate = 0;
    unsigned long currentMillis = millis();

    if (PowerManager::getCurrentSleepState() == PowerManager::AWAKE) {
        RoverManager::updateHoverAnimation();
        drawSprite();
    }
    
    if (currentMillis - lastDisplayUpdate >= 50) {
        LOG_SCOPE("Main loop update cycle");
        PowerManager::checkSleepState();
        
        switch (PowerManager::getCurrentSleepState()) {
            case PowerManager::AWAKE:
                LOG_DEBUG("Display state: AWAKE");
                setBacklight(255);
                drawSprite();
                updateLEDs();
                break;
                
            case PowerManager::DIM_DISPLAY:
                LOG_DEBUG("Display state: DIM");
                setBacklight(64);
                drawSprite();
                updateLEDs();
                break;
                
            case PowerManager::DISPLAY_OFF:
                LOG_DEBUG("Display state: OFF");
                tft.writecommand(TFT_DISPOFF);
                setBacklight(0);
                updateLEDs();
                break;
                
            case PowerManager::DEEP_SLEEP:
                LOG_DEBUG("Display state: DEEP SLEEP");
                goToSleep();
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
