#ifndef ROVER_VIEW_MANAGER_H
#define ROVER_VIEW_MANAGER_H

#include "TFT_eSPI.h"
#include "utilities.h"

// Forward declaration
extern TFT_eSprite spr;

class RoverViewManager {
public:
    enum ViewType {
        TODO_LIST,
        QUOTES,
        WEATHER,
        STATS,
        // Add more view types here as needed
    };

    static void init();
    static void setCurrentView(ViewType view);
    static void nextView();
    static void previousView();
    static void drawCurrentView();
    static ViewType getCurrentView() { return currentView; }

private:
    static const int FRAME_X = 2;
    static const int FRAME_Y = 195;
    static const int FRAME_WIDTH = 280;
    static const int FRAME_HEIGHT = 120;
    static const int TITLE_Y_OFFSET = 15;
    static const uint16_t FRAME_COLOR = 0xC618;
    static const uint16_t FRAME_BORDER_COLOR = TFT_DARKGREY;
    
    static ViewType currentView;
    
    // Drawing methods for different views
    static void drawFrame();
    static void drawTodoList();
    static void drawQuotes();
    static void drawWeather();
    static void drawStats();
};

#endif 