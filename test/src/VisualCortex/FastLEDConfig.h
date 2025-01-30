#ifndef FASTLED_CONFIG_H
#define FASTLED_CONFIG_H

// Core FastLED Configuration
#define FASTLED_INTERNAL
#define FASTLED_ESP32_I2S 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_RMT_MAX_CHANNELS 2
#define FASTLED_ESP32_RMT 1

// Visual Cortex specific LED configurations
#define VISUAL_CORTEX_MAX_BRIGHTNESS 255
#define VISUAL_CORTEX_DEFAULT_FPS 60
#define VISUAL_CORTEX_COLOR_CORRECTION TypicalLEDStrip
#define VISUAL_CORTEX_LED_TYPE WS2812B

// Animation timing constants
#define VISUAL_CORTEX_LOADING_DELAY 100
#define VISUAL_CORTEX_FLASH_DURATION 100
#define VISUAL_CORTEX_FADE_STEP 5
#define VISUAL_CORTEX_MIN_FADE 50
#define VISUAL_CORTEX_MAX_FADE 250
#define VISUAL_CORTEX_ERROR_MIN_FADE 64

// Pattern-specific configurations
#define VISUAL_CORTEX_LEDS_PER_STEP 3
#define VISUAL_CORTEX_ERROR_LED_INDEX 0
#define VISUAL_CORTEX_ERROR_LED_COUNT 8
#define VISUAL_CORTEX_FADE_INCREMENT 5
#define VISUAL_CORTEX_ANIMATION_DELAY 50

// Boot sequence timing
#define VISUAL_CORTEX_BOOT_STEP_DELAY 100
#define VISUAL_CORTEX_SUCCESS_FLASH_DURATION 100
#define VISUAL_CORTEX_ERROR_FLASH_DURATION 200

#endif 