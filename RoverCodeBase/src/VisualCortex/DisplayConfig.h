/*
 * DisplayConfig.h
 * 
 * Display configuration constants for the visual cortex.
 * Defines screen dimensions, center points, and frame offsets
 * used across the display rendering system.
 */

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

namespace VisualCortex 
{

    // Screen dimensions and center points
    #define SCREEN_WIDTH 240
    #define SCREEN_HEIGHT 320
    #define SCREEN_CENTER_X (SCREEN_WIDTH / 2)
    #define SCREEN_CENTER_Y (SCREEN_HEIGHT / 2) 

    #define FRAME_OFFSET_X 40

}

#endif // DISPLAY_CONFIG_H