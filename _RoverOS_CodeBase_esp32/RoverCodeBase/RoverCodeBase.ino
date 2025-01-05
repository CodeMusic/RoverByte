#include <Wire.h>
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
        currentSleepState = AWAKE;
        tft.writecommand(TFT_DISPON);
        setBacklight(255);
        FastLED.setBrightness(50);
        FastLED.show();
        drawSprite();
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
    static int lastPos = 0;
    int newPos = encoder.getPosition();
    
    if (newPos != lastPos) {
        PowerManager::wakeFromSleep();
        
        if (newPos > lastPos) {
            RoverManager::nextMood();  // Make sure this is being called
            // Ascending tones for right turn
            playTone(1047, 50);
            delay(10);
            playTone(1319, 50);
            delay(10);
            playTone(1568, 50);
        } else {
            RoverManager::previousMood();  // Make sure this is being called
            // Descending tones for left turn
            playTone(1568, 50);
            delay(10);
            playTone(1319, 50);
            delay(10);
            playTone(1047, 50);
        }
        lastPos = newPos;
        drawSprite();  // Make sure we redraw after mood change
    }

    // Handle button press
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(ENCODER_KEY);
    
    if (buttonState != lastButtonState) {
        delay(50); // Simple debounce
        if (buttonState == LOW) { // Button pressed
            showTime = true;  // Enable time display on button press
            isWeekMode = !isWeekMode;  // Toggle between week and full view
            updateLEDs();  // Update LED display with new mode
            drawSprite();  // Refresh display with new mode
            
            // Play different tones for each mode
            if (isWeekMode) {
                playTone(1000, 100);
            } else {
                playTone(800, 100);
            }
        }
        lastButtonState = buttonState;
    }
}


void drawSprite() {
    spr.fillSprite(TFT_BLACK);
    
    // Draw status bar at top
    drawStatusBar();
    
    // Get current time
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Show small rover with time when showTime is true and not in dim display state
    bool showSmallWithTime = showTime && (currentSleepState == AWAKE);
    
    // Draw Rover - large when !showTime, small when showTime
    RoverManager::drawRover(
        RoverManager::getCurrentMood(),
        RoverManager::earsPerked,
        !showSmallWithTime,  // Reversed logic: large when NOT showing time
        tft.width() / 2,
        showSmallWithTime ? 50 : 80
    );
    
    spr.pushSprite(0, 0);
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

void drawStatusBar() {
    int barY = 160;
    int barHeight = 160;
    int statusX = SCREEN_CENTER_X + 27;
    
    // Get current time for color
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Draw outer rectangle with month colors
    CRGB color1, color2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, color1, color2);
    uint16_t monthColor1 = ColorUtilities::convertToRGB565(color1);
    uint16_t monthColor2 = ColorUtilities::convertToRGB565(color2);
    
    if (timeInfo->tm_mon == timeInfo->tm_mon) {
        spr.fillRect(0, barY, tft.width(), barHeight, monthColor1);
    } else {
        for (int i = 0; i < tft.width(); i++) {
            uint8_t mix = (i * 255) / tft.width();
            uint16_t color = spr.alphaBlend(mix, monthColor1, monthColor2);
            spr.drawFastVLine(i, barY, barHeight, color);
        }
    }
    
    // Draw date number with smaller font
    spr.setTextFont(2);  // Changed to font 2 for smaller size
    spr.setTextColor(TFT_BLACK);
    char dateStr[3];
    sprintf(dateStr, "%d", timeInfo->tm_mday);
    spr.drawString(dateStr, 25, barY + 15);
    
    // Draw black status area
    spr.fillRect(50, barY - 2, 145, 35, TFT_BLACK);
    
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
            spr.drawString("Level: 21", statusX, barY + 15);
            break;
        case 1:
            spr.drawString("XP: 1337/2000", statusX, barY + 15);
            break;
        case 2:
            // Update battery info using PowerManager
            String batteryText = String(PowerManager::getBatteryPercentage()) + "% ";
            if (PowerManager::isCharging()) {
                batteryText += PowerManager::getChargeStatus();
            }
            spr.drawString(batteryText, statusX, barY + 15);
            break;
    }
    
    // Draw todo list section
    spr.fillRect(2, 195, 280, 120, 0xC618);
    spr.drawRect(4, 193, 284, 124, TFT_DARKGREY);
    
    // Draw todo items
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, 0xC618);
    spr.drawString("Today's Tasks:", SCREEN_CENTER_X, 210);
    
    spr.setTextFont(2);
    // Sample todo items (these could be made dynamic later)
    spr.drawString("1. Migrate Home", SCREEN_CENTER_X, 225);
    spr.drawString("2. Organize My Place", SCREEN_CENTER_X, 245);
    spr.drawString("3. Update Rover", SCREEN_CENTER_X, 265);
}

void updateLEDs() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    if (isWeekMode) {
        // LED 0: Month color blinking at 25% brightness
        CRGB color1, color2;
        ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, color1, color2);
        if (timeInfo->tm_sec % 2 == 0) {
            color1.nscale8(64);  // 25% brightness
            leds[0] = color1;
        } else {
            leds[0] = CRGB::Black;
        }
        
        // LEDs 1-7: Days of week
        for (int i = 0; i < 7; i++) {
            CRGB dayColor = ColorUtilities::getDayColor(i + 1);
            
            if (i < timeInfo->tm_wday) {
                // Past days - 0% brightness
                leds[i + 1] = CRGB::Black;
            } 
            else if (i == timeInfo->tm_wday) {
                // Current day - blinking at 91% brightness
                if (timeInfo->tm_sec % 2 == 0) {
                    dayColor.nscale8(232);  // 91% brightness
                    leds[i + 1] = dayColor;
                } else {
                    leds[i + 1] = CRGB::Black;
                }
            }
            else {
                // Future days - 70% brightness
                dayColor.nscale8(179);  // 70% brightness
                leds[i + 1] = dayColor;
            }
        }
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
    static unsigned long lastDisplayUpdate = 0;
    unsigned long currentMillis = millis();
    
    if (PowerManager::getCurrentSleepState() == PowerManager::AWAKE) {
        RoverManager::updateHoverAnimation();  // Make sure this is called
        drawSprite();
    }
    
    readEncoder();

    if (currentMillis - lastDisplayUpdate >= 50) {
        LOG_SCOPE("Main loop update cycle");
        handleSideButton();
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
                tft.writecommand(TFT_DISPOFF);
                setBacklight(0);
                fill_solid(leds, WS2812_NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
        }
        
        lastDisplayUpdate = currentMillis;
    }
    
    if (isPlayingSound) {
        audio.loop();
    }
    
    tryWiFiConnection();
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
