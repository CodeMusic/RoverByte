#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 8  // Adjust this value (try 4, 8, or 16)
#include <FastLED.h>
#include "LEDManager.h"
#include <time.h>
#include "ColorUtilities.h"
#include "../PrefrontalCortex/utilities.h"
#include "../AuditoryCortex/SoundFxManager.h"

// Static member initialization
CRGB LEDManager::leds[WS2812_NUM_LEDS];
Mode LEDManager::currentMode = Mode::FULL_MODE;
Mode LEDManager::previousMode = Mode::FULL_MODE;
uint8_t LEDManager::currentPosition = 0;
uint8_t LEDManager::currentColorIndex = 0;
uint8_t LEDManager::completedCycles = 0;
uint8_t LEDManager::filledPositions = 0;
uint8_t LEDManager::activeTrails = 0;
unsigned long LEDManager::lastStepTime = 0;
bool LEDManager::isLoading = false;
FestiveTheme LEDManager::currentTheme = FestiveTheme::CHRISTMAS;
uint8_t LEDManager::animationStep = 0;
uint8_t LEDManager::fadeValue = 128;
bool LEDManager::fadeDirection = true;
CRGB LEDManager::previousColors[WS2812_NUM_LEDS];

void LEDManager::init() {
    pinMode(BOARD_PWR_EN, OUTPUT);
    digitalWrite(BOARD_PWR_EN, HIGH);  // Power on LEDs

    FastLED.addLeds<WS2813, WS2812_DATA_PIN, GRB>(leds, WS2812_NUM_LEDS);
    FastLED.setBrightness(50);
    FastLED.clear(true);
    FastLED.show();
    delay(50);

    startLoadingAnimation();  // Start in loading state
}

void LEDManager::stopLoadingAnimation() {
    if (!isLoading) return;  // Guard against multiple calls
    
    isLoading = false;
    FastLED.clear();
    FastLED.show();
    
    // Only set mode and start updates if time is synchronized
    if (time(nullptr) > 1000000000) {
        currentMode = Mode::FULL_MODE;
        updateLEDs();
    } else {
        // If time isn't synced, keep LEDs off
        FastLED.clear();
        FastLED.show();
    }
}

void LEDManager::updateLEDs() {
    static unsigned long lastUpdate = 0;
    const unsigned long UPDATE_INTERVAL = 125;  // 125ms update interval
    
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentTime;
        
        if (isLoading) {
            updateLoadingAnimation();
        } else {
            switch (currentMode) {
                case Mode::FULL_MODE:
                    updateFullMode();
                    break;
                case Mode::WEEK_MODE:
                    updateWeekMode();
                    break;
                case Mode::TIMER_MODE:
                    updateTimerMode();
                    break;
                case Mode::OFF_MODE:
                    FastLED.clear();
                    FastLED.show();
                    break;
                case Mode::FESTIVE_MODE:
                    updateFestiveMode();
                    break;
            }
            FastLED.show();
        }
    }
}

void LEDManager::setMode(Mode newMode) {
    currentMode = newMode;
    FastLED.clear();
    updateLEDs();
}

void LEDManager::nextMode() {
    currentMode = static_cast<Mode>((static_cast<int>(currentMode) + 1) % LED_NUM_MODES);
    FastLED.clear();
    updateLEDs();
}

void LEDManager::updateFullMode() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Create a 3-state blink cycle (0, 1, 2) using integer division of seconds
    int blinkState = (timeInfo->tm_sec % 3);
    
    // LED 0: Current day color (1-7, where 1 is Sunday)
    CRGB dayColor = ColorUtilities::getDayColor(timeInfo->tm_wday + 1);
    leds[0] = dayColor;
    
    // LED 1: Week number (base 5)
    int weekNum = (timeInfo->tm_mday + 6) / 7;
    switch(weekNum) {
        case 1: leds[1] = CRGB::Red; break;
        case 2: leds[1] = CRGB::Orange; break;
        case 3: leds[1] = CRGB::Yellow; break;
        case 4: leds[1] = CRGB::Green; break;
        default: leds[1] = CRGB::Blue; break;
    }
    
    // LED 2: Month (base 12)
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    if (monthColor1 == monthColor2) {
        leds[2] = monthColor1;
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
    ColorUtilities::getHourColors(hour12, hourColor1, hourColor2);
    if (hourColor1 == hourColor2) {
        leds[3] = hourColor1;
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
    int day = timeInfo->tm_mday;
    int dayTens = day / 8;
    int dayOnes = day % 8;
    leds[6] = ColorUtilities::getBase8Color(dayOnes);
    leds[7] = ColorUtilities::getBase8Color(dayTens);
}

void LEDManager::updateWeekMode() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    
    // Create a common blink state for both LEDs
    bool shouldBlink = (timeInfo->tm_sec % 2 == 0);
    
    // LED 0: Month color with alternating pattern
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    
    // Month LED should always blink
    if (shouldBlink) {
        monthColor1.nscale8(MONTH_DIM);
        leds[0] = monthColor1;
    } else {
        leds[0] = CRGB::Black;
    }
    
    // Days of week
    for (int i = 1; i <= 7; i++) {
        CRGB dayColor = ColorUtilities::getDayColor(i);
        
        if (i - 1 < timeInfo->tm_wday) {
            leds[i] = CRGB::Black;  // Past days are off
        } else if (i - 1 == timeInfo->tm_wday) {
            // Current day should always blink
            if (shouldBlink) {
                dayColor.nscale8(184);  // Bright when on
                leds[i] = dayColor;
            } else {
                leds[i] = CRGB::Black;  // Off during blink
            }
        } else {
            dayColor.nscale8(77);  // Future days are dimmed
            leds[i] = dayColor;
        }
    }
}

void LEDManager::updateTimerMode() {
    static const CRGB timerColors[] = {
        CRGB::Red, CRGB::Orange, CRGB::Yellow, 
        CRGB::Green, CRGB::Blue, CRGB::Indigo, 
        CRGB::Purple, CRGB::White, CRGB::Black
    };
    static const int NUM_TIMER_COLORS = sizeof(timerColors) / sizeof(timerColors[0]);
    static CRGB backgroundColors[WS2812_NUM_LEDS] = {CRGB::Black};
    static bool isMoving = false;
    
    unsigned long currentTime = millis();
    if (currentTime - lastStepTime >= 125) {  // 125ms between moves
        lastStepTime = currentTime;
        
        if (!isMoving) {
            // Start new drop at position 0
            leds[0] = timerColors[currentColorIndex];
            currentPosition = 0;
            isMoving = true;
        } else {
            // Clear current position
            leds[currentPosition] = backgroundColors[currentPosition];
            
            // Move to next position if possible
            if (currentPosition < WS2812_NUM_LEDS - 1 && 
                leds[currentPosition + 1] == backgroundColors[currentPosition + 1]) {
                currentPosition++;
                leds[currentPosition] = timerColors[currentColorIndex];
            } else {
                // Drop has reached its final position
                leds[currentPosition] = timerColors[currentColorIndex];
                SoundFxManager::playTimerDropSound(timerColors[currentColorIndex]);
                isMoving = false;
                
                // Check if we completed this color's cycle
                bool allFilled = true;
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    if (leds[i] == backgroundColors[i]) {
                        allFilled = false;
                        break;
                    }
                }
                
                if (allFilled) {
                    // Update background for next color
                    for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                        backgroundColors[i] = timerColors[currentColorIndex];
                    }
                    currentColorIndex = (currentColorIndex + 1) % NUM_TIMER_COLORS;
                    if (currentColorIndex == 0) {
                        // Reset background when starting over
                        for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                            backgroundColors[i] = CRGB::Black;
                        }
                    }
                }
            }
        }
        FastLED.show();
    }
}

CRGB LEDManager::getRainbowColor(uint8_t index) {
    static const CRGB rainbowColors[] = {
        CRGB::Red,
        CRGB::Orange,
        CRGB::Yellow,
        CRGB::Green,
        CRGB::Blue,
        CRGB(75, 0, 130),   // Indigo
        CRGB(148, 0, 211)   // Violet
    };
    return rainbowColors[index % NUM_RAINBOW_COLORS];
}

void LEDManager::startLoadingAnimation() {
    currentColorIndex = 0;
    currentPosition = 0;
    filledPositions = 0;
    completedCycles = 0;
    lastStepTime = 0;
    isLoading = true;
    FastLED.clear();
    FastLED.show();
}

void LEDManager::updateLoadingAnimation() {
    if (!isLoading) return;
    
    FastLED.clear();
    // Set Canadian flag pattern
    leds[0] = CRGB::Red;
    leds[4] = CRGB::Red;
    leds[1] = CRGB::White;
    leds[2] = CRGB::White;
    leds[3] = CRGB::White;
    leds[5] = CRGB::White;
    leds[6] = CRGB::White;
    leds[7] = CRGB::White;
    FastLED.show();
}

bool LEDManager::isLoadingComplete() {
    return completedCycles >= WS2812_NUM_LEDS;
}

void LEDManager::setLED(int index, CRGB color) {
    if (index >= 0 && index < WS2812_NUM_LEDS) {
        // Direct color setting without conversion
        leds[index] = color;
    }
}

void LEDManager::showLEDs() {
    FastLED.show();
}

void LEDManager::scaleLED(int index, uint8_t scale) {
    if (index >= 0 && index < WS2812_NUM_LEDS) {
        leds[index].nscale8(scale);
    }
}

void LEDManager::flashSuccess() {
    // Save current LED state
    CRGB savedState[WS2812_NUM_LEDS];
    memcpy(savedState, leds, sizeof(CRGB) * WS2812_NUM_LEDS);
    
    // Flash green
    fill_solid(leds, WS2812_NUM_LEDS, CRGB::Green);
    FastLED.show();
    delay(100);
    
    // Restore previous state
    memcpy(leds, savedState, sizeof(CRGB) * WS2812_NUM_LEDS);
    FastLED.show();
}

void LEDManager::flashLevelUp() {
    // First pass - clockwise light up in gold/yellow
    for (int i = 0; i < WS2812_NUM_LEDS; i++) {
        leds[i] = CRGB::Gold;
        FastLED.show();
        delay(50);
    }
    
    // Flash all bright white
    for (int i = 0; i < WS2812_NUM_LEDS; i++) {
        leds[i] = CRGB::White;
    }
    FastLED.show();
    delay(100);
    
    // Sparkle effect
    for (int j = 0; j < 3; j++) {  // Do 3 sparkle cycles
        for (int i = 0; i < WS2812_NUM_LEDS; i++) {
            if (random(2) == 0) {  // 50% chance for each LED
                leds[i] = CRGB::Gold;
            } else {
                leds[i] = CRGB::White;
            }
        }
        FastLED.show();
        delay(100);
    }
    
    // Fade out
    for (int i = 0; i < WS2812_NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
        FastLED.show();
        delay(30);
    }
}

void LEDManager::displayCardPattern(uint8_t* uid, uint8_t length) {
    // Save current LED state
    CRGB savedLeds[WS2812_NUM_LEDS];
    memcpy(savedLeds, leds, sizeof(CRGB) * WS2812_NUM_LEDS);
    
    // Display UID pattern
    for (int i = 0; i < ((int)length < 8 ? (int)length : 8); i++) {
        uint8_t value = uid[i];
        CRGB color;
        color.r = value & 0x3;  // 2 bits for red
        color.g = (value >> 2) & 0x7;  // 3 bits for green
        color.b = (value >> 5) & 0x7;  // 3 bits for blue
        
        // Scale up the colors
        color.r = color.r * 64;  // 0-255 range
        color.g = color.g * 32;
        color.b = color.b * 32;
        
        leds[i] = color;
    }
    FastLED.show();
    
    // Restore original state after 2 seconds
    delay(2000);
    memcpy(leds, savedLeds, sizeof(CRGB) * WS2812_NUM_LEDS);
    FastLED.show();
}

void LEDManager::syncLEDsForDay() {
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int currentDay = timeInfo->tm_wday;
    int currentHour = timeInfo->tm_hour % 12;
    if (currentHour == 0) currentHour = 12;
    
    static const CRGB hourColors[] = {
        CRGB::Red, CRGB(255, 69, 0), CRGB::Orange,
        CRGB(255, 165, 0), CRGB::Yellow, CRGB::Green,
        CRGB::Blue, CRGB(75, 0, 130), CRGB(75, 0, 130),
        CRGB(75, 0, 130), CRGB(148, 0, 211), CRGB::Purple
    };

    setLED(0, hourColors[currentHour - 1]);
    scaleLED(0, 178);  // 70% brightness
    
    for (int i = 1; i <= 7; i++) {
        setLED(i, CRGB::White);
        scaleLED(i, i <= currentDay ? 128 : 28);
    }
    
    showLEDs();
}

void LEDManager::update() {
    if (isLoading) {
        updateLoadingAnimation();
        return;
    }

    switch (currentMode) {
        case Mode::FULL_MODE:
            updateFullMode();
            break;
        case Mode::WEEK_MODE:
            updateWeekMode();
            break;
        case Mode::TIMER_MODE:
            updateTimerMode();
            break;
        case Mode::OFF_MODE:
            FastLED.clear();
            FastLED.show();
            break;
        case Mode::FESTIVE_MODE:
            updateFestiveMode();
            break;
    }
}

void LEDManager::updateFestiveMode() {
    unsigned long currentTime = millis();
    if (currentTime - lastStepTime >= 50) {  // 50ms animation interval
        lastStepTime = currentTime;
        
        switch (currentTheme) {
            case FestiveTheme::NEW_YEAR:
                // Sparkle effect with gold and white
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    if (random8() < 50) { // 20% chance to change each LED
                        leds[i] = random8() > 127 ? CRGB::Gold : CRGB::White;
                    }
                }
                break;
                
            case FestiveTheme::VALENTINES:
                // Pulsing hearts effect
                fadeValue = fadeValue + (fadeDirection ? 5 : -5);
                if (fadeValue >= 250) fadeDirection = false;
                if (fadeValue <= 50) fadeDirection = true;
                
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    CRGB color = (i % 2 == 0) ? CRGB::Red : CRGB::Pink;
                    color.nscale8(fadeValue);
                    leds[i] = color;
                }
                break;
                
            case FestiveTheme::ST_PATRICK:
                // Rotating shamrock effect
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    int adjustedPos = (i + animationStep) % WS2812_NUM_LEDS;
                    switch (adjustedPos % 3) {
                        case 0: leds[i] = CRGB::Green; break;
                        case 1: leds[i] = CRGB(0, 180, 0); break;
                        case 2: leds[i] = CRGB(0, 100, 0); break;
                    }
                }
                animationStep = (animationStep + 1) % WS2812_NUM_LEDS;
                break;
                
            case FestiveTheme::EASTER:
                // Soft pastel fade between colors
                if (++animationStep >= 255) {
                    animationStep = 0;
                    currentColorIndex = (currentColorIndex + 1) % 4;
                }
                
                CRGB targetColor;
                switch (currentColorIndex) {
                    case 0: targetColor = CRGB::Pink; break;
                    case 1: targetColor = CRGB(255, 255, 150); break;
                    case 2: targetColor = CRGB(150, 255, 255); break;
                    case 3: targetColor = CRGB(200, 255, 200); break;
                }
                
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    leds[i] = blend(previousColors[i], targetColor, animationStep);
                    previousColors[i] = leds[i];
                }
                break;
                
            case FestiveTheme::CANADA_DAY:
                // Waving flag effect
                animationStep = (animationStep + 1) % 255;
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    int wave = sin8(animationStep + (i * 32));
                    if (i == 0 || i == 4) {
                        CRGB red = CRGB::Red;
                        red.nscale8(wave);
                        leds[i] = red;
                    } else {
                        CRGB white = CRGB::White;
                        white.nscale8(wave);
                        leds[i] = white;
                    }
                }
                break;
                
            case FestiveTheme::HALLOWEEN:
                // Spooky fade between orange and purple
                fadeValue = fadeValue + (fadeDirection ? 5 : -5);
                if (fadeValue >= 250) fadeDirection = false;
                if (fadeValue <= 50) fadeDirection = true;
                
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    CRGB color = (i % 2 == 0) ? CRGB::Orange : CRGB::Purple;
                    color.nscale8(fadeValue);
                    leds[i] = color;
                }
                break;
                
            case FestiveTheme::CHRISTMAS:
                // Twinkling lights effect
                for (int i = 0; i < WS2812_NUM_LEDS; i++) {
                    CRGB color = (i % 2 == 0) ? CRGB::Red : CRGB::Green;
                    if (random8() < 20) { // 8% chance to twinkle
                        color.nscale8(random8(128, 255));
                    } else {
                        color.nscale8(128);
                    }
                    leds[i] = color;
                }
                break;
        }
        FastLED.show();
    }
}

void LEDManager::setFestiveTheme(FestiveTheme theme) {
    currentTheme = theme;
    currentMode = Mode::FESTIVE_MODE;
    FastLED.clear();
    updateLEDs();
} 