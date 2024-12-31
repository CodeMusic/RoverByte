#include <Wire.h>
#include <FastLED.h>
#include "TFT_eSPI.h"
#include <RotaryEncoder.h>
#include <time.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include "utilities.h"
#include "Audio.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);


#define CLOCK_PIN 45
CRGB leds[WS2812_NUM_LEDS];

RotaryEncoder encoder(ENCODER_INA, ENCODER_INB, RotaryEncoder::LatchMode::TWO03);


#define color1 0xC638
#define color2  0xC638

int value=980;
bool muted=0;
int deb=0;

// Add color definitions for days of the week
#define SUNDAY    0xF800    // Red
#define MONDAY    0xFD20    // Orange
#define TUESDAY   0xFFE0    // Yellow
#define WEDNESDAY 0x07E0    // Green
#define THURSDAY  0x001F    // Blue
#define FRIDAY    0x4810    // Indigo
#define SATURDAY  0x780F    // Violet

// Add color definitions for month eyes
const uint16_t monthColors[][2] = {
    {0xF800, 0xF800},  // January   - Red/Red
    {0xF800, 0xFD20},  // February  -    Red/Orange
    {0xFD20, 0xFD20},  // March     - Orange/Orange
    {0xFD20, 0xFFE0},  // April     -    Orange/Yellow
    {0xFFE0, 0xFFE0},  // May       - Yellow/Yellow
    {0x07E0, 0x07E0},  // June      - Green/Green
    {0x07E0, 0x001F},  // July      -    Green/Blue
    {0x001F, 0x001F},  // August    - Blue/Blue
    {0x001F, 0x4810},  // September -    Blue/Indigo
    {0x4810, 0x4810},  // October   - Indigo/Indigo
    {0x4810, 0x780F},  // November  -    Indigo/Violet
    {0x780F, 0x780F}   // December  - Violet/Violet
};

const char* ssid = "CodeMusicai";
const char* password = "cnatural";

#define SCREEN_CENTER_X 85  // Adjust this value to shift everything left or right

// Add at top with other global variables
unsigned long lastCounterUpdate = 0;
const unsigned long COUNTER_SPEED = 1000;  // 1 second interval
unsigned long lastAnimationStep = 0;
const unsigned long ANIMATION_DELAY = 250;  // 250ms between animation steps
bool isAnimating = false;
int animationStep = 0;
const int TOTAL_ANIMATION_STEPS = 14;  // 7 steps for turning off + 7 steps for turning on

// Add at top with other global variables
const char* moods[] = {"happy", "sad", "cool", "sleeping", "looking_left", "looking_right", "intense", "broken"};
int currentMood = 0;
int numMoods = 8;  // Number of moods in the array
static int pos = 0;
bool earsPerked = false;

// Add near top with other defines
#define I2S_BCK_PIN  BOARD_VOICE_BCLK   // Pin 46
#define I2S_WS_PIN   BOARD_VOICE_LRCLK  // Pin 40
#define I2S_DOUT_PIN BOARD_VOICE_DIN    // Pin 7

// Single declaration of Audio
Audio audio;
bool isPlayingSound = false;

// Add at top with other global variables
bool isPlayingAuldLangSyne = false;
int currentNote = 0;

// Add at top with other global variables
int roverYOffset = 0;
bool movingDown = true;
const int MAX_OFFSET = 5;  // Maximum pixels to move up/down
unsigned long lastHoverUpdate = 0;
const int HOVER_SPEED = 90;  // Update every 100ms

// Add at top with other global variables
unsigned long lastStatusUpdate = 0;
const unsigned long STATUS_CHANGE_INTERVAL = 3000;  // Switch every 3 seconds
bool showLevel = true;  // Toggle between level and experience

// Add at top with other global variables
const CRGB BASE_8_COLORS[] = {
    CRGB::Black,    // 0 = Off
    CRGB::Red,      // 1
    CRGB(255, 140, 0),  // 2 = Orange
    CRGB::Yellow,   // 3
    CRGB::Green,    // 4
    CRGB::Blue,     // 5
    CRGB(75, 0, 130),   // 6 = Indigo
    CRGB(148, 0, 211)   // 7 = Violet
};

const CRGB MONTH_COLORS[][2] = {
    {CRGB::Red, CRGB::Red},                    // January (Red)
    {CRGB::Red, CRGB(255, 140, 0)},           // February (Red/Orange)
    {CRGB(255, 140, 0), CRGB(255, 140, 0)},   // March (Orange)
    {CRGB(255, 140, 0), CRGB::Yellow},        // April (Orange/Yellow)
    {CRGB::Yellow, CRGB::Yellow},             // May (Yellow)
    {CRGB::Green, CRGB::Green},               // June (Green)
    {CRGB::Green, CRGB::Blue},                // July (Green/Blue)
    {CRGB::Blue, CRGB::Blue},                 // August (Blue)
    {CRGB::Blue, CRGB(75, 0, 130)},           // September (Blue/Indigo)
    {CRGB(75, 0, 130), CRGB(75, 0, 130)},     // October (Indigo)
    {CRGB(75, 0, 130), CRGB(148, 0, 211)},    // November (Indigo/Violet)
    {CRGB(148, 0, 211), CRGB(148, 0, 211)}    // December (Violet)
};

const CRGB DAY_COLORS[] = {
    CRGB::Red,           // Sunday
    CRGB(255, 140, 0),  // Monday (Orange)
    CRGB::Yellow,       // Tuesday
    CRGB::Green,        // Wednesday
    CRGB::Blue,         // Thursday
    CRGB(75, 0, 130),   // Friday (Indigo)
    CRGB(148, 0, 211)   // Saturday (Violet)
};

void setup() {
    Serial.begin(115200);
    
    // Initialize side button first
    pinMode(BOARD_IR_EN, INPUT_PULLUP);
    delay(100); // Give time for pin to stabilize
    
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
    audio.setPinout(I2S_BCK_PIN, I2S_WS_PIN, I2S_DOUT_PIN);
    audio.setVolume(21);
    Serial.println("Audio initialized");
    
    Wire.begin(43,44);
    

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

    // Connect to WiFi and get time
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    
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
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected");
        
        // Configure time for Eastern Time (-4 for EDT)
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // -4 hours from GMT, no DST offset
        
        // Wait for time sync with visual feedback
        Serial.println("Waiting for time sync...");
        int attempts = 0;
        while (time(nullptr) < 1000000000 && attempts < 30) {  // 30 attempts max
            delay(1000);
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
            Serial.println("\nTime synchronized");
        } else {
            Serial.println("\nTime sync failed!");
        }
    } else {
        Serial.println("\nWiFi connection failed!");
    }
    
    // Initialize LEDs
    FastLED.addLeds<WS2812B, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    updateLEDs();  // Initial LED update
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

void playAuldLangSyne() {
    // Notes for Auld Lang Syne (shifted up three octaves)
    static const int melody[] = {
        3136, 3520, 3136, 2637, 3136, 3520, 3136,  // G7 A7 G7 E7 G7 A7 G7
        4186, 3136, 3520, 3136, 2637, 2349, 2637,  // C8 G7 A7 G7 E7 D7 E7
        3136, 3520, 3136, 2637, 3136, 3520, 3136,  // G7 A7 G7 E7 G7 A7 G7
        4186, 3136, 3520, 3136, 2637, 2349, 2637   // C8 G7 A7 G7 E7 D7 E7
    };
    
    static const int durations[] = {
        250, 250, 375, 125, 250, 250, 500,
        250, 250, 375, 125, 250, 250, 500,
        250, 250, 375, 125, 250, 250, 500,
        250, 250, 375, 125, 250, 250, 500
    };

    if (isPlayingAuldLangSyne) {
        playTone(melody[currentNote], durations[currentNote]);
        delay(durations[currentNote] * 0.3); // Short pause between notes
        
        currentNote++;
        if (currentNote >= sizeof(melody) / sizeof(melody[0])) {
            currentNote = 0;
            isPlayingAuldLangSyne = false;
        }
    }
}

void readEncoder() {
    static int pos = 0;
    encoder.tick();

    if(digitalRead(ENCODER_KEY) == 0) {
        if(deb == 0) {
            deb = 1;
            
            // Toggle song playback
            if (isPlayingAuldLangSyne) {
                isPlayingAuldLangSyne = false;  // Stop playing
                currentNote = 0;  // Reset to beginning
            } else {
                isPlayingAuldLangSyne = true;  // Start playing
            }
            
            drawSprite();
        }
    } else {
        deb = 0;
    }
    
    int newPos = encoder.getPosition();
    bool forward = newPos > pos;
    if (pos != newPos) {
        if(newPos > pos) {
            currentMood = (currentMood + 1) % numMoods;  // Cycle forward through moods
        }
        if(newPos < pos) {
            currentMood = (currentMood - 1 + numMoods) % numMoods;  // Cycle backward through moods
        }
        
        pos = newPos;
        drawSprite();
        
        // Check if the MP3 file exists
        if (SPIFFS.exists("/move.mp3")) 
        {
            audio.connecttoFS(SPIFFS, "/move.mp3");
        } else {
            // Generate a beep if the file doesn't exist
            if(forward) 
            {
                playTone(1000, 200);  // 1000 Hz for 200 ms
                playTone(2000, 200);  // 2000 Hz for 200 ms
            }
            else 
            {
                playTone(2000, 200);  // 2000 Hz for 200 ms
                playTone(1000, 200);  // 1000 Hz for 200 ms
            }
        }
    }
}


void drawSprite() {
    spr.fillSprite(TFT_BLACK);
    
    // Get current time and update LEDs
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Set time color and update LEDs
    uint16_t timeColor;
    switch(timeInfo->tm_wday) {
        case 0: timeColor = SUNDAY; break;
        case 1: timeColor = MONDAY; break;
        case 2: timeColor = TUESDAY; break;
        case 3: timeColor = WEDNESDAY; break;
        case 4: timeColor = THURSDAY; break;
        case 5: timeColor = FRIDAY; break;
        case 6: timeColor = SATURDAY; break;
        default: timeColor = TFT_WHITE; break;
    }
    
    //updateWeekLEDs();
    
    // Draw time at top (centered)
    char timeStr[6];
    int hour12 = (timeInfo->tm_hour % 12) ? (timeInfo->tm_hour % 12) : 12;
    sprintf(timeStr, "%02d:%02d", hour12, timeInfo->tm_min);
    
    spr.setTextColor(timeColor, TFT_BLACK);
    spr.drawString(timeStr, SCREEN_CENTER_X, 30, 7);
    
    // Draw Rover in middle (centered)
    drawRover(moods[currentMood], earsPerked);
    
    // Draw status bar (which now includes the countdown)
    drawStatusBar();
    
    spr.pushSprite(0, 0);
}

void drawRover(String mood, bool earsPerked) {
    // Get current month for eye colors
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    uint16_t leftEyeColor = monthColors[timeInfo->tm_mon][0];
    uint16_t rightEyeColor = monthColors[timeInfo->tm_mon][1];
    
    int roverX = SCREEN_CENTER_X - 50;  // Center point for Rover
    int baseY = 80;  // Base Y position
    
    // Apply hover animation offset
    int currentY = baseY + roverYOffset;
    
    // Draw Rover's body (white)
    spr.fillRect(roverX, currentY, 100, 70, TFT_WHITE);
    
    // Draw ears (triangles) with adjusted Y position
    if (earsPerked) {
        // Perked ears
        spr.fillTriangle(roverX + 10, currentY - 20, roverX + 25, currentY - 5, roverX + 40, currentY - 20, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, currentY - 20, roverX + 75, currentY - 5, roverX + 90, currentY - 20, TFT_WHITE);  // Right ear
    } else {
        // Normal ears
        spr.fillTriangle(roverX + 10, currentY - 10, roverX + 25, currentY + 5, roverX + 40, currentY - 10, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, currentY - 10, roverX + 75, currentY + 5, roverX + 90, currentY - 10, TFT_WHITE);  // Right ear
    }
    
    // Draw eye panel (silver rectangle)
    spr.fillRect(roverX + 15, currentY + 5, 70, 30, 0xC618);
    
    if (mood == "cool") {
        // Draw sunglasses with adjusted Y
        spr.fillRect(roverX + 20, currentY + 15, 60, 15, TFT_BLACK);  // Sunglasses bar
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_BLACK);   // Left lens
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_BLACK);   // Right lens
        spr.drawLine(roverX + 25, currentY + 15, roverX + 30, currentY + 15, TFT_WHITE);
        spr.drawLine(roverX + 65, currentY + 15, roverX + 70, currentY + 15, TFT_WHITE);
    } else {
        // Normal eyes with adjusted Y
        spr.fillCircle(roverX + 30, currentY + 20, 10, TFT_WHITE);  // Left eye white
        spr.fillCircle(roverX + 70, currentY + 20, 10, TFT_WHITE);  // Right eye white
        
        if (mood == "sleeping") {
            // Closed eyes
            spr.drawLine(roverX + 25, currentY + 20, roverX + 35, currentY + 20, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 20, roverX + 75, currentY + 20, TFT_BLACK);
        } else if (mood == "looking_left") {
            // Eyes looking left
            spr.fillCircle(roverX + 25, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 65, currentY + 20, 5, rightEyeColor);
        } else if (mood == "looking_right") {
            // Eyes looking right
            spr.fillCircle(roverX + 35, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 75, currentY + 20, 5, rightEyeColor);
        } else if (mood == "intense") {
            // Intense eyes (smaller)
            spr.fillCircle(roverX + 30, currentY + 20, 3, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 3, rightEyeColor);
        } else if (mood == "broken") {
            // X eyes
            spr.drawLine(roverX + 25, currentY + 15, roverX + 35, currentY + 25, TFT_BLACK);
            spr.drawLine(roverX + 25, currentY + 25, roverX + 35, currentY + 15, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 15, roverX + 75, currentY + 25, TFT_BLACK);
            spr.drawLine(roverX + 65, currentY + 25, roverX + 75, currentY + 15, TFT_BLACK);
        } else {
            // Default eyes (happy/neutral)
            spr.fillCircle(roverX + 30, currentY + 20, 5, leftEyeColor);
            spr.fillCircle(roverX + 70, currentY + 20, 5, rightEyeColor);
        }
    }
    
    // Draw nose with adjusted Y
    spr.fillTriangle(roverX + 45, currentY + 35, roverX + 40, currentY + 45, roverX + 50, currentY + 45, TFT_BLACK);
    
    // Draw mouth with adjusted Y
    spr.drawLine(roverX + 50, currentY + 45, roverX + 50, currentY + 55, TFT_BLACK);
    
    if (mood == "happy") {
        spr.drawArc(roverX + 50, currentY + 55, 15, 10, 270, 450, TFT_BLACK, TFT_BLACK);
    } else if (mood == "sad") {
        spr.drawArc(roverX + 50, currentY + 70, 20, 15, 180, 360, TFT_BLACK, TFT_BLACK);
    } else if (mood == "intense") {
        spr.drawLine(roverX + 35, currentY + 60, roverX + 65, currentY + 60, TFT_BLACK);
    } else if (mood == "sleeping") {
        spr.drawArc(roverX + 50, currentY + 60, 15, 10, 0, 180, TFT_BLACK, TFT_BLACK);
    }
}

void updateWeekLEDs() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int currentDay = timeInfo->tm_wday;  // 0 = Sunday, 6 = Saturday
    
    // Set the colors for each day
    CRGB dayColors[] = {
        CRGB::Red,           // Sunday
        CRGB(255, 140, 0),  // Monday (Orange)
        CRGB::Yellow,       // Tuesday
        CRGB::Green,        // Wednesday
        CRGB::Blue,         // Thursday
        CRGB(75, 0, 130),   // Friday (Indigo)
        CRGB(148, 0, 211)   // Saturday (Violet)
    };
    
    // Update the first 7 LEDs based on remaining days
    for (int i = 0; i < 7; i++) {
        if (i <= currentDay) {
            // Turn off past days
            leds[i] = CRGB::Black;
        } else {
            // Light up remaining days with their colors
            leds[i] = dayColors[i];
        }
    }
    
    // Current day should be lit
    if (currentDay > 0) {  // If not Sunday
        leds[currentDay] = dayColors[currentDay];
    }
    
    // Set the 8th LED to white
    leds[7] = CRGB::White;
    
    FastLED.show();
}

void handleSideButton() {
    static bool lastState = HIGH;  // Initialize to HIGH (not pressed)
    static unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = 50;
    
    bool currentState = digitalRead(BOARD_IR_EN);
    
    // Debounce
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (currentState != lastState) {
            lastDebounceTime = millis();
            lastState = currentState;
            
            Serial.print("Side button state changed to: ");
            Serial.println(currentState ? "HIGH" : "LOW");
            
            if (currentState == LOW) {  // Button pressed
                if (!earsPerked) {
                    Serial.println("Perking ears and playing sound...");
                    earsPerked = true;
                    drawSprite();

                    if (SD.exists("/bark.mp3")) {
                        Serial.println("Playing bark sound...");
                        audio.connecttoFS(SD, "/bark.mp3");
                        isPlayingSound = true;
                        delay(10);
                    } else {
                        Serial.println("bark.mp3 not found! Playing fallback tone...");
                        playTone(300, 100);
                        delay(50);
                        playTone(400, 100);
                    }
                }
            } else {  // Button released
                if (earsPerked) {
                    Serial.println("Returning ears to normal...");
                    earsPerked = false;
                    drawSprite();
                }
            }
        }
    }
}

void playTone(int frequency, int duration) {
    isPlayingSound = true;  // Set flag to true when sound starts

    // Set LED color based on the closest musical note
    if (frequency >= 1047) {        // High C (C6)
        leds[0] = CRGB::Red;
    } else if (frequency >= 988) {  // B5
        leds[0] = CRGB(148, 0, 211);  // Violet
    } else if (frequency >= 880) {  // A5
        leds[0] = CRGB(75, 0, 130);   // Indigo
    } else if (frequency >= 784) {  // G5
        leds[0] = CRGB::Blue;
    } else if (frequency >= 698) {  // F5
        leds[0] = CRGB::Green;
    } else if (frequency >= 659) {  // E5
        leds[0] = CRGB::Yellow;
    } else if (frequency >= 587) {  // D5
        leds[0] = CRGB(255, 140, 0);  // Orange
    } else {                        // C5 or lower
        leds[0] = CRGB::Red;
    }
    FastLED.show();

    // Configure a PWM channel
    ledcSetup(0, frequency, 8);  // Channel 0, frequency, 8-bit resolution
    ledcAttachPin(BOARD_VOICE_DIN, 0);  // Attach the channel to the speaker pin

    // Play the tone
    ledcWriteTone(0, frequency);  // Set the frequency
    delay(duration);  // Play for the specified duration

    // Stop the tone
    ledcWriteTone(0, 0);

    isPlayingSound = false;  // Reset flag after sound stops
}

void drawStatusBar() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Calculate time until 2025
    struct tm target = {0};
    target.tm_year = 125;  // 2025 (years since 1900)
    target.tm_mon = 0;     // January
    target.tm_mday = 1;    // 1st
    target.tm_hour = 0;    // Midnight
    target.tm_min = 0;
    target.tm_sec = 0;
    
    time_t targetTime = mktime(&target);
    double diff = difftime(targetTime, now);
    
    // Calculate countdown components
    int hoursLeft = (int)((diff / 3600)) % 24;
    int minsLeft = (int)((diff / 60)) % 60;
    int secsLeft = (int)diff % 60;
    
    int barY = 160;  // Position below Rover
    int barHeight = 160;  // Extend all the way to bottom
    int dateWidth = 45;
    
    // Draw outer rectangle with month colors (full width)
    if (timeInfo->tm_mon == timeInfo->tm_mon) {
        spr.fillRect(0, barY, tft.width(), barHeight, monthColors[timeInfo->tm_mon][0]);
    } else {
        for (int i = 0; i < tft.width(); i++) {
            uint8_t mix = (i * 255) / tft.width();
            uint16_t color = spr.alphaBlend(mix, monthColors[timeInfo->tm_mon][0], monthColors[timeInfo->tm_mon][1]);
            spr.drawFastVLine(i, barY, barHeight, color);
        }
    }
    
    // Draw date number (larger, like a tab)
    spr.setTextFont(4);  // Larger font for date
    spr.setTextColor(TFT_BLACK);
    char dateStr[3];
    sprintf(dateStr, "%d", timeInfo->tm_mday);
    spr.drawString(dateStr, 25, barY + 15);  // Adjusted position for larger font
    
    // Draw black status area (taller and more to the left, moved up)
    spr.fillRect(45, barY - 2, 140, 35, TFT_BLACK);  // Moved up by adjusting Y position
    
    // Draw status text (centered in black area)
    spr.setTextFont(2);
    spr.setTextColor(TFT_WHITE);
    if (showLevel) {
        spr.drawString("Level: 1", 110, barY + 15);  // Adjusted for new black area position
    } else {
        spr.drawString("Experience: 100", 110, barY + 15);  // Adjusted for new black area position
    }
    
    // Draw 2025 countdown section (moved up, made taller)
    spr.fillRect(2, 195, 280, 120, 0xC618);  // Moved up and made taller
    spr.drawRect(4, 193, 284, 124, TFT_DARKGREY);  // Adjusted border to match
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, 0xC618);
    spr.drawString("2025 in", SCREEN_CENTER_X, 215);  // Moved up
    
    // Draw seconds in large font (back to center)
    char secsStr[10];
    sprintf(secsStr, "%d", (int)diff);
    spr.setTextFont(7);
    spr.drawString(secsStr, SCREEN_CENTER_X, 255);  // Moved up
    
    // Draw hours and minutes in smaller font
    spr.setTextFont(2);
    char countdownStr[50];
    sprintf(countdownStr, "%d Hours, %d Minutes", hoursLeft, minsLeft);
    spr.drawString(countdownStr, SCREEN_CENTER_X, 285);  // Moved up
}

void updateLEDs() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate < 1000) return; // Update every second
    lastUpdate = millis();

    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Debug print current time
    Serial.printf("Time: %02d:%02d:%02d\n", 
        timeInfo->tm_hour, 
        timeInfo->tm_min, 
        timeInfo->tm_sec);
    
    // LED 0: Month color (possibly gradient)
    int month = timeInfo->tm_mon;
    leds[0] = MONTH_COLORS[month][timeInfo->tm_sec % 2];
    Serial.printf("Month LED: %d (Color1: R%d G%d B%d", month, 
        MONTH_COLORS[month][timeInfo->tm_sec % 2].r,
        MONTH_COLORS[month][timeInfo->tm_sec % 2].g,
        MONTH_COLORS[month][timeInfo->tm_sec % 2].b);
    
    // LED 1: Day of week color
    int dayOfWeek = timeInfo->tm_wday;
    leds[1] = DAY_COLORS[dayOfWeek];
    Serial.printf("Day LED: %d (R%d G%d B%d)\n", dayOfWeek,
        DAY_COLORS[dayOfWeek].r,
        DAY_COLORS[dayOfWeek].g,
        DAY_COLORS[dayOfWeek].b);
    
    // Calculate seconds since midnight
    long secondsSinceMidnight = timeInfo->tm_hour * 3600L + 
                               timeInfo->tm_min * 60L + 
                               timeInfo->tm_sec;
    
    Serial.printf("Seconds since midnight: %ld\n", secondsSinceMidnight);
    
    // Convert to base 8 for last 6 LEDs (right to left)
    long temp = secondsSinceMidnight;
    for (int i = 7; i >= 2; i--) {
        int digit = temp % 8;
        leds[i] = BASE_8_COLORS[digit];
        temp /= 8;
        Serial.printf("LED %d: digit %d (R%d G%d B%d)\n", i, digit,
            BASE_8_COLORS[digit].r,
            BASE_8_COLORS[digit].g,
            BASE_8_COLORS[digit].b);
    }
    
    // Force LED update
    FastLED.show();
    delay(1);  // Small delay to ensure LED update
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    static unsigned long lastStatusUpdate = 0;
    unsigned long currentMillis = millis();
    
    // Update LEDs every second
    updateLEDs();
    
    // Update display every 50ms to keep clock ticking
    if (currentMillis - lastDisplayUpdate >= 50) {
        readEncoder();
        handleSideButton();
        drawSprite();  // This will update the clock display
        lastDisplayUpdate = currentMillis;
    }
    
    // Update status text every 3 seconds
    if (currentMillis - lastStatusUpdate >= STATUS_CHANGE_INTERVAL) {
        showLevel = !showLevel;
        lastStatusUpdate = currentMillis;
    }
    
    if (isPlayingAuldLangSyne) {
        playAuldLangSyne();
    }
    
    if (isPlayingSound) {
        audio.loop();
    }
}
