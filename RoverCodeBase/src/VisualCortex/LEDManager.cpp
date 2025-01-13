#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_SPI_BUS FSPI
#define FASTLED_ESP32_SPI_CLOCK_DIVIDER 4  // Adjust this value (try 4, 8, or 16)
#include <FastLED.h>
#include "LEDManager.h"
#include <time.h>
#include "ColorUtilities.h"
#include "../PrefrontalCortex/utilities.h"

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
    if (isLoading) {
        LOG_DEBUG("Still in loading state");
        updateLoadingAnimation();
    } else {
        LOG_DEBUG("Current mode: %d", static_cast<int>(currentMode));
        FastLED.clear();  // Clear previous state
        switch (currentMode) {
            case Mode::FULL_MODE:
                if (currentMode != previousMode) {
                    LOG_DEBUG("Updating FULL_MODE");
                }
                updateFullMode();
                previousMode = currentMode;
                break;
            case Mode::WEEK_MODE:
                if (currentMode != previousMode) {
                    LOG_DEBUG("Updating WEEK_MODE");
                }
                updateWeekMode();
                previousMode = currentMode;
                break;
            case Mode::TIMER_MODE:
                if (currentMode != previousMode) {
                    LOG_DEBUG("Updating TIMER_MODE");
                }
                updateTimerMode();
                previousMode = currentMode;
                break;
        }
        FastLED.show();
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
    
    // LED 0: Month color with alternating pattern
    CRGB monthColor1, monthColor2;
    ColorUtilities::getMonthColors(timeInfo->tm_mon + 1, monthColor1, monthColor2);
    
    int blinkState = (timeInfo->tm_sec % 3);
    
    if (monthColor1 == monthColor2) {
        if (timeInfo->tm_sec % 2 == 0) {
            monthColor1.nscale8(MONTH_DIM);
            leds[0] = monthColor1;
        } else {
            leds[0] = CRGB::Black;
        }
    } else {
        switch(blinkState) {
            case 0:
                monthColor1.nscale8(MONTH_DIM);
                leds[0] = monthColor1;
                break;
            case 1:
                monthColor2.nscale8(MONTH_DIM);
                leds[0] = monthColor2;
                break;
            case 2:
                leds[0] = CRGB::Black;
                break;
        }
    }
    
    // Days of week
    for (int i = 1; i <= 7; i++) {
        CRGB dayColor = ColorUtilities::getDayColor(i);
        
        if (i - 1 < timeInfo->tm_wday) {
            leds[i] = CRGB::Black;
        } else if (i - 1 == timeInfo->tm_wday) {
            if (timeInfo->tm_sec % 2 == 0) {
                dayColor.nscale8(184);
                leds[i] = dayColor;
            } else {
                leds[i] = CRGB::Black;
            }
        } else {
            dayColor.nscale8(77);
            leds[i] = dayColor;
        }
    }
}

void LEDManager::updateTimerMode() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastStepTime >= STEP_DELAY) {
        lastStepTime = currentTime;
        
        CRGB currentColor = getRainbowColor(currentColorIndex);
        
        // Move current color down one position
        if (currentPosition < WS2812_NUM_LEDS) {
            // Clear previous position if not at bottom
            if (currentPosition > 0) {
                leds[currentPosition - 1] = CRGB::Black;
            }
            
            // Set new position
            leds[currentPosition] = currentColor;
            currentPosition++;
            
            // When reaching bottom
            if (currentPosition >= WS2812_NUM_LEDS) {
                // Keep the color at the bottom
                leds[WS2812_NUM_LEDS - 1] = currentColor;
                
                // Start next drop
                currentPosition = 0;
                filledPositions++;
                
                // After 8 drops of same color
                if (filledPositions >= WS2812_NUM_LEDS) {
                    // Move to next color
                    currentColorIndex = (currentColorIndex + 1) % NUM_RAINBOW_COLORS;
                    filledPositions = 0;
                }
            }
            FastLED.show();
        }
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