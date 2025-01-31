/**
 * @brief Visual perception configuration and display parameters
 * 
 * Defines the visual processing parameters for the cognitive display system:
 * - Screen dimensions and boundaries
 * - Color perception mappings
 * - Visual feedback thresholds
 * - Font configurations for cognitive output
 * - Layout specifications for visual cortex
 * 
 * The DisplayConfig serves as the foundational visual processing framework,
 * coordinating:
 * - Visual field dimensions
 * - Perceptual color spaces
 * - Text processing parameters
 * - Spatial organization
 * - Visual feedback patterns
 */

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <TFT_eSPI.h>
#include "../CorpusCallosum/SynapticPathways.h"

namespace VisualCortex 
{
    /**
     * @brief Core visual processing parameters
     * Defines fundamental visual perception constants
     */
    struct DisplayConfig 
    {
        // Visual field boundaries
        static constexpr int16_t SCREEN_WIDTH = 170;
        static constexpr int16_t SCREEN_HEIGHT = 320;
        static constexpr int16_t SCREEN_CENTER_X = SCREEN_WIDTH / 2;
        static constexpr int16_t SCREEN_CENTER_Y = SCREEN_HEIGHT / 2;

        // Frame boundaries
        static constexpr int16_t FRAME_WIDTH = 150;
        static constexpr int16_t FRAME_HEIGHT = 280;
        static constexpr int16_t FRAME_OFFSET_X = (SCREEN_WIDTH - FRAME_WIDTH) / 2;
        static constexpr int16_t FRAME_OFFSET_Y = (SCREEN_HEIGHT - FRAME_HEIGHT) / 2;

        // Text processing parameters
        static constexpr uint8_t MENU_FONT = 2;
        static constexpr uint8_t STATUS_FONT = 2;
        static constexpr uint8_t TITLE_FONT = 4;
        
        // Visual spacing thresholds
        static constexpr uint8_t MENU_ITEM_HEIGHT = 30;
        static constexpr uint8_t MENU_PADDING = 10;
        static constexpr uint8_t TEXT_PADDING = 5;
        
        // Color perception values
        static constexpr uint16_t BACKGROUND_COLOR = TFT_BLACK;
        static constexpr uint16_t TEXT_COLOR = TFT_WHITE;
        static constexpr uint16_t HIGHLIGHT_COLOR = TFT_BLUE;
        static constexpr uint16_t ERROR_COLOR = TFT_RED;
        
        // Visual feedback thresholds
        static constexpr uint16_t ANIMATION_DELAY = 50;
        static constexpr uint8_t MAX_MENU_ITEMS = 8;
        static constexpr uint8_t SCROLL_THRESHOLD = 4;
    };
}

#endif // DISPLAY_CONFIG_H