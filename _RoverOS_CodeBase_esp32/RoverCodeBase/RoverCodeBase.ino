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
const char* moods[] = {"happy", "sad", "cool", "sleeping", "looking_left", "looking_right", "intense", "broken"};
int currentMood = 0;
int numMoods = 8;  // Number of moods in the array
static int pos = 0;
bool earsPerked = false;

Audio audio;

bool isPlayingSound = false;  // Global flag to track sound playing

void setup() {
  
  pinMode(46, OUTPUT);
  digitalWrite(46, HIGH);

  tft.begin();
  tft.setRotation(0);  // Set the desired rotation
  tft.fillScreen(TFT_BLACK);

  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  pinMode(0, INPUT_PULLUP);
  FastLED.addLeds<APA102, WS2812_DATA_PIN, CLOCK_PIN, RGB>(leds, WS2812_NUM_LEDS);

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
  FastLED.setBrightness(50);
  FastLED.show();
  drawSprite();

  // Add WiFi and time sync
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  
  // Configure time for Eastern Standard Time (EST)
  configTime(-18000, 0, "pool.ntp.org", "time.nist.gov");  // -5 hours for EST
  // For Eastern Daylight Time (EDT), use -14400 (4 hours behind UTC)

  syncLEDsForDay();
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
    static int pos = 0;
    encoder.tick();

    if(digitalRead(ENCODER_KEY) == 0) {
        if(deb == 0) {
            deb = 1;
            muted = !muted;
            drawSprite();
            delay(200);
        }
    } else {
        deb = 0;
    }
    
    int newPos = encoder.getPosition();
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
        if (SPIFFS.exists("/move.mp3")) {
            audio.connecttoFS(SPIFFS, "/move.mp3");
        } else {
            // Generate a beep if the file doesn't exist
            playTone(1000, 200);  // 1000 Hz for 200 ms
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
    
    updateWeekLEDs();
    
    // Draw time at top (centered) in 12-hour format
    char timeStr[6];
    int hour12 = (timeInfo->tm_hour % 12) ? (timeInfo->tm_hour % 12) : 12;
    sprintf(timeStr, "%02d:%02d", hour12, timeInfo->tm_min);
    
    spr.setTextColor(timeColor, TFT_BLACK);
    spr.drawString(timeStr, SCREEN_CENTER_X, 30, 7);  // Adjust Y position for top half
    
    // Draw Rover in middle (centered)
    drawRover(moods[currentMood], earsPerked);
    
    // Conditionally draw "Beep" if sound is playing
    if (isPlayingSound) {
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.setTextFont(2);
        spr.drawString("Beep", SCREEN_CENTER_X, 110);  // Adjust Y position as needed
    }
    
    // Draw Level and Experience between Rover and ToDo list
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.setTextFont(2);
    spr.drawString("Level: 1", SCREEN_CENTER_X - 40, 170);  // Adjust X and Y positions as needed
    spr.drawString("Exp: 100", SCREEN_CENTER_X + 40, 170);  // Adjust X and Y positions as needed
    
    // Draw ToDo section in the bottom half with 3D effect
    spr.fillRect(20, 200, 280, 110, 0xC618);  // Adjust Y position and dimensions
    spr.drawRect(18, 198, 284, 114, TFT_DARKGREY);  // Add a border for 3D effect
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, 0xC618);
    spr.drawString("ToDo:", SCREEN_CENTER_X, 220);  // Adjust Y position as needed
    spr.setTextFont(2);
    spr.drawString("Pick up Eggs", SCREEN_CENTER_X, 250);  // Adjust Y position as needed
    
    spr.pushSprite(0, 0);  // Push sprite to the top-left corner
}

void drawRover(String mood, bool earsPerked) {
    // Get current month for eye colors
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    uint16_t leftEyeColor = monthColors[timeInfo->tm_mon][0];
    uint16_t rightEyeColor = monthColors[timeInfo->tm_mon][1];
    
    int roverX = SCREEN_CENTER_X - 50;  // Center point for Rover
    
    // Draw Rover's body (white)
    spr.fillRect(roverX, 80, 100, 70, TFT_WHITE);
    
    // Draw ears (triangles)
    if (earsPerked) {
        // Perked ears
        spr.fillTriangle(roverX + 10, 60, roverX + 25, 75, roverX + 40, 60, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, 60, roverX + 75, 75, roverX + 90, 60, TFT_WHITE);  // Right ear
    } else {
        // Normal ears
        spr.fillTriangle(roverX + 10, 70, roverX + 25, 85, roverX + 40, 70, TFT_WHITE);  // Left ear
        spr.fillTriangle(roverX + 60, 70, roverX + 75, 85, roverX + 90, 70, TFT_WHITE);  // Right ear
    }
    
    // Draw eye panel (silver rectangle)
    spr.fillRect(roverX + 15, 85, 70, 30, 0xC618);
    
    if (mood == "cool") {
        // Draw sunglasses
        spr.fillRect(roverX + 20, 95, 60, 15, TFT_BLACK);  // Sunglasses bar
        spr.fillCircle(roverX + 30, 100, 10, TFT_BLACK);   // Left lens
        spr.fillCircle(roverX + 70, 100, 10, TFT_BLACK);   // Right lens
        // Add shine to glasses
        spr.drawLine(roverX + 25, 95, roverX + 30, 95, TFT_WHITE);
        spr.drawLine(roverX + 65, 95, roverX + 70, 95, TFT_WHITE);
    } else {
        // Normal eyes
        spr.fillCircle(roverX + 30, 100, 10, TFT_WHITE);  // Left eye white
        spr.fillCircle(roverX + 70, 100, 10, TFT_WHITE);  // Right eye white
        
        if (mood == "sleeping") {
            // Closed eyes
            spr.drawLine(roverX + 25, 100, roverX + 35, 100, TFT_BLACK);
            spr.drawLine(roverX + 65, 100, roverX + 75, 100, TFT_BLACK);
        } else if (mood == "looking_left") {
            // Eyes looking left
            spr.fillCircle(roverX + 25, 100, 5, leftEyeColor);
            spr.fillCircle(roverX + 65, 100, 5, rightEyeColor);
        } else if (mood == "looking_right") {
            // Eyes looking right
            spr.fillCircle(roverX + 35, 100, 5, leftEyeColor);
            spr.fillCircle(roverX + 75, 100, 5, rightEyeColor);
        } else if (mood == "intense") {
            // Intense eyes (smaller)
            spr.fillCircle(roverX + 30, 100, 3, leftEyeColor);
            spr.fillCircle(roverX + 70, 100, 3, rightEyeColor);
        } else if (mood == "broken") {
            // X eyes
            spr.drawLine(roverX + 25, 95, roverX + 35, 105, TFT_BLACK);
            spr.drawLine(roverX + 25, 105, roverX + 35, 95, TFT_BLACK);
            spr.drawLine(roverX + 65, 95, roverX + 75, 105, TFT_BLACK);
            spr.drawLine(roverX + 65, 105, roverX + 75, 95, TFT_BLACK);
        } else {
            // Default eyes (happy/neutral)
            spr.fillCircle(roverX + 30, 100, 5, leftEyeColor);
            spr.fillCircle(roverX + 70, 100, 5, rightEyeColor);
        }
    }
    
    // Draw nose
    spr.fillTriangle(roverX + 45, 115, roverX + 40, 125, roverX + 50, 125, TFT_BLACK);
    
    // Draw mouth line and expression
    spr.drawLine(roverX + 50, 125, roverX + 50, 135, TFT_BLACK);
    
    if (mood == "happy") {
        // Rotate the arc to make it look like a smile
        spr.drawArc(roverX + 50, 135, 15, 10, 270, 450, TFT_BLACK, TFT_BLACK);  // Adjusted for a smile
    } else if (mood == "sad") {
        spr.drawArc(roverX + 50, 150, 20, 15, 180, 360, TFT_BLACK, TFT_BLACK);
    } else if (mood == "intense") {
        // Straight determined line
        spr.drawLine(roverX + 35, 140, roverX + 65, 140, TFT_BLACK);
    } else if (mood == "sleeping") {
        // Slight smile while sleeping
        spr.drawArc(roverX + 50, 140, 15, 10, 0, 180, TFT_BLACK, TFT_BLACK);
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
    pinMode(BOARD_IR_EN, INPUT_PULLUP);

    if (digitalRead(BOARD_IR_EN) == LOW) {
        if (!earsPerked) {
            earsPerked = true;
            drawSprite();  // Redraw with ears perked

            // Check if the recording file exists
            if (SPIFFS.exists("/record.wav")) {
                audio.connecttoFS(SPIFFS, "/record.wav");
            } else {
                // Generate a beep if the file doesn't exist
                playTone(1000, 200);  // Use BOARD_VOICE_DIN for sound
            }
        }
    } else {
        if (earsPerked) {
            earsPerked = false;
            drawSprite();  // Redraw with normal ears

            // Stop recording and play back
            audio.stopSong();
            if (SPIFFS.exists("/record.wav")) {
                audio.connecttoFS(SPIFFS, "/record.wav");
            } else {
                // Generate a beep if the file doesn't exist
                playTone(1000, 200);  // Use BOARD_VOICE_DIN for sound
            }
        }
    }
}

void playTone(int frequency, int duration) {
    isPlayingSound = true;  // Set flag to true when sound starts

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

void loop() {
    readEncoder();
    handleSideButton();
}