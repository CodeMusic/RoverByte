#include "RoverViewManager.h"

// Initialize static members
RoverViewManager::ViewType RoverViewManager::currentView = RoverViewManager::TODO_LIST;

void RoverViewManager::init() {
    LOG_PROD("Initializing RoverViewManager");
    currentView = TODO_LIST;
}

void RoverViewManager::setCurrentView(ViewType view) {
    LOG_DEBUG("Changing view to: %d", view);
    currentView = view;
}

void RoverViewManager::nextView() {
    int nextView = (static_cast<int>(currentView) + 1) % 4; // Update this when adding new views
    currentView = static_cast<ViewType>(nextView);
    LOG_DEBUG("Next view selected: %d", currentView);
}

void RoverViewManager::previousView() {
    int prevView = (static_cast<int>(currentView) - 1 + 4) % 4; // Update this when adding new views
    currentView = static_cast<ViewType>(prevView);
    LOG_DEBUG("Previous view selected: %d", currentView);
}

void RoverViewManager::drawCurrentView() {
    LOG_SCOPE("Drawing view: %d", currentView);
    
    drawFrame();
    
    switch (currentView) {
        case TODO_LIST:
            drawTodoList();
            break;
        case QUOTES:
            drawQuotes();
            break;
        case WEATHER:
            drawWeather();
            break;
        case STATS:
            drawStats();
            break;
    }
}

void RoverViewManager::drawFrame() {
    LOG_SCOPE("Drawing view frame");
    spr.fillRect(FRAME_X, FRAME_Y, FRAME_WIDTH, FRAME_HEIGHT, FRAME_COLOR);
    spr.drawRect(FRAME_X + 2, FRAME_Y - 2, FRAME_WIDTH + 4, FRAME_HEIGHT + 4, FRAME_BORDER_COLOR);
}

void RoverViewManager::drawTodoList() {
    LOG_SCOPE("Drawing TODO list view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Today's Tasks:", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    spr.drawString("1. Migrate Home", SCREEN_CENTER_X, FRAME_Y + 30);
    spr.drawString("2. Organize My Place", SCREEN_CENTER_X, FRAME_Y + 50);
    spr.drawString("3. Update Rover", SCREEN_CENTER_X, FRAME_Y + 70);
}

void RoverViewManager::drawQuotes() {
    LOG_SCOPE("Drawing quotes view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Quote of the Day", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    // You could have an array of quotes and randomly select one
    spr.drawString("\"The best way to predict", SCREEN_CENTER_X, FRAME_Y + 40);
    spr.drawString("the future is to create it.\"", SCREEN_CENTER_X, FRAME_Y + 60);
    spr.drawString("- Peter Drucker", SCREEN_CENTER_X, FRAME_Y + 80);
}

void RoverViewManager::drawWeather() {
    LOG_SCOPE("Drawing weather view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Weather", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    // This could be updated with real weather data
    spr.drawString("Sunny", SCREEN_CENTER_X, FRAME_Y + 40);
    spr.drawString("72°F / 22°C", SCREEN_CENTER_X, FRAME_Y + 60);
    spr.drawString("Humidity: 45%", SCREEN_CENTER_X, FRAME_Y + 80);
}

void RoverViewManager::drawStats() {
    LOG_SCOPE("Drawing stats view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("System Stats", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    // This could show various system statistics
    spr.drawString("Uptime: 3d 12h", SCREEN_CENTER_X, FRAME_Y + 40);
    spr.drawString("Memory: 65% free", SCREEN_CENTER_X, FRAME_Y + 60);
    spr.drawString("WiFi: Connected", SCREEN_CENTER_X, FRAME_Y + 80);
} 