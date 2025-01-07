#include "RoverViewManager.h"

// Initialize static members
RoverViewManager::ViewType RoverViewManager::currentView = RoverViewManager::VIRTUES;

// Forward declare all drawing functions
void drawRootChakra(int x, int y, int size);
void drawSacralChakra(int x, int y, int size);
void drawSolarChakra(int x, int y, int size);
void drawHeartChakra(int x, int y, int size);
void drawThroatChakra(int x, int y, int size);
void drawThirdEyeChakra(int x, int y, int size);
void drawCrownChakra(int x, int y, int size);
void drawChastitySymbol(int x, int y, int size);
void drawTemperanceSymbol(int x, int y, int size);
void drawCharitySymbol(int x, int y, int size);
void drawDiligenceSymbol(int x, int y, int size);
void drawForgivenessSymbol(int x, int y, int size);
void drawKindnessSymbol(int x, int y, int size);
void drawHumilitySymbol(int x, int y, int size);

// Update array names to match header
const RoverViewManager::ChakraInfo RoverViewManager::CHAKRA_DATA[] = {
    {"Root Chakra", "Survival, Grounding, Stability, Comfort, Safety", TFT_RED, drawRootChakra},
    {"Sacral Chakra", "Sensuality, Sexuality, Pleasure, Creativity, Emotions", 0xFDA0, drawSacralChakra},
    {"Solar Plexus Chakra", "Strength, Ego, Power, Self-esteem, Digestion", 0xFFE0, drawSolarChakra},
    {"Heart Chakra", "Love, Acceptance, Compassion, Kindness, Peace", 0x07E0, drawHeartChakra},
    {"Throat Chakra", "Communication, Expression, Honesty, Purification", 0x001F, drawThroatChakra},
    {"Third Eye Chakra", "Intuition, Visualization, Imagination, Clairvoyance", 0x180E, drawThirdEyeChakra},
    {"Crown Chakra", "Knowledge, Fulfillment, Spirituality, Reality", 0x780F, drawCrownChakra}
};

const RoverViewManager::VirtueInfo RoverViewManager::VIRTUE_DATA[] = {
    {"Chastity cures Lust", "Acting purely quells excessive sexual appetites", TFT_RED, drawChastitySymbol},
    {"Temperance cures Gluttony", "Practicing self-restraint quells over-indulgence", 0xFDA0, drawTemperanceSymbol},
    {"Charity cures Greed", "Giving quells avarice", 0xFFE0, drawCharitySymbol},
    {"Diligence cures Sloth", "Your integrity and attention to detail quells laziness", 0x07E0, drawDiligenceSymbol},
    {"Forgiveness cures Wrath", "Practice keeping your composure to quell anger", 0x001F, drawForgivenessSymbol},
    {"Kindness cures Envy", "Practice admiration to quell jealousy", 0x180E, drawKindnessSymbol},
    {"Humility cures Pride", "Show humbleness to quell vanity", 0x780F, drawHumilitySymbol}
};

void RoverViewManager::init() {
    LOG_PROD("Initializing RoverViewManager");
}

void RoverViewManager::setCurrentView(ViewType view) {
    LOG_DEBUG("Changing view to: %d", view);
    currentView = view;
}

void RoverViewManager::nextView() {
    LOG_DEBUG("Changing from view %d to next view", currentView);
    currentView = static_cast<ViewType>((currentView + 1) % ViewType::NUM_VIEWS);
    LOG_DEBUG("New view is %d", currentView);
    drawCurrentView();
}

void RoverViewManager::previousView() {
    LOG_DEBUG("Changing from view %d to previous view", currentView);
    currentView = static_cast<ViewType>((currentView - 1 + ViewType::NUM_VIEWS) % ViewType::NUM_VIEWS);
    LOG_DEBUG("New view is %d", currentView);
    drawCurrentView();
}

void RoverViewManager::drawCurrentView() {
    LOG_DEBUG("Drawing view %d", currentView);
    
    // Draw the frame first
    drawFrame();
    
    // Set common text properties - changed to black text on frame color background
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    
    switch (currentView) {
        case ViewType::TODO_LIST:
            LOG_DEBUG("Drawing TODO_LIST view");
            drawTodoList();
            break;
        case ViewType::CHAKRAS:
            LOG_DEBUG("Drawing CHAKRAS view");
            spr.setTextFont(2);  // Smaller font
            drawChakras();
            break;
        case ViewType::VIRTUES:
            LOG_DEBUG("Drawing VIRTUES view");
            spr.setTextFont(2);  // Smaller font
            drawVirtues();
            break;
        case ViewType::QUOTES:
            LOG_DEBUG("Drawing QUOTES view");
            drawQuotes();
            break;
        case ViewType::WEATHER:
            LOG_DEBUG("Drawing WEATHER view");
            drawWeather();
            break;
        case ViewType::STATS:
            LOG_DEBUG("Drawing STATS view");
            drawStats();
            break;
        case ViewType::NEXTMEAL:
            LOG_DEBUG("Drawing NEXTMEAL view");
            drawNextMeal();
            break;
        default:
            LOG_DEBUG("Unknown view %d", currentView);
            break;
    }
    
    spr.pushSprite(0, 0);
}

void RoverViewManager::drawFrame() {
    LOG_SCOPE("Drawing view frame");
    spr.fillRect(FRAME_X, FRAME_Y + 5, FRAME_WIDTH, FRAME_HEIGHT - 5, FRAME_COLOR);  // Move frame down 5 pixels
    spr.drawRect(FRAME_X + 2, FRAME_Y + 3, FRAME_WIDTH + 4, FRAME_HEIGHT - 1, FRAME_BORDER_COLOR);
}

void RoverViewManager::drawTodoList() {
    LOG_SCOPE("Drawing TODO list view");
    
    spr.setTextFont(4);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString("Today's Tasks:", SCREEN_CENTER_X, FRAME_Y + TITLE_Y_OFFSET);
    
    spr.setTextFont(2);
    spr.drawString("1. Service Canada", SCREEN_CENTER_X, FRAME_Y + 30);
    spr.drawString("2. Call Doctor", SCREEN_CENTER_X, FRAME_Y + 50);
    spr.drawString("3. Call Therapist", SCREEN_CENTER_X, FRAME_Y + 70);
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

// Define symbols for each virtue
void drawChastitySymbol(int x, int y, int size) {
    // Pure white lily
    spr.drawCircle(x, y, size/2, TFT_RED);
    spr.drawLine(x, y - size/2, x, y + size/2, TFT_RED);
    spr.drawLine(x - size/2, y, x + size/2, y, TFT_RED);
}

void drawTemperanceSymbol(int x, int y, int size) {
    // Balanced scales
    spr.drawLine(x - size/2, y, x + size/2, y, 0xFDA0);
    spr.fillCircle(x - size/3, y - size/4, size/6, 0xFDA0);
    spr.fillCircle(x + size/3, y - size/4, size/6, 0xFDA0);
}

void drawCharitySymbol(int x, int y, int size) {
    // Heart with hands
    spr.fillCircle(x - size/4, y - size/4, size/4, 0xFFE0);
    spr.fillCircle(x + size/4, y - size/4, size/4, 0xFFE0);
    spr.fillTriangle(x, y + size/3, x - size/2, y - size/6, x + size/2, y - size/6, 0xFFE0);
}

void drawDiligenceSymbol(int x, int y, int size) {
    // Bee or honeycomb pattern
    for(int i = 0; i < 6; i++) {
        float angle = i * PI / 3;
        int x1 = x + cos(angle) * size/2;
        int y1 = y + sin(angle) * size/2;
        spr.drawLine(x, y, x1, y1, 0x07E0);
    }
}

void drawForgivenessSymbol(int x, int y, int size) {
    // Dove or peaceful wave
    spr.drawCircle(x, y, size/2, 0x001F);
    spr.drawCircle(x, y, size/3, 0x001F);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
}

void drawKindnessSymbol(int x, int y, int size) {
    // Open hands or gift
    spr.drawRect(x - size/2, y - size/2, size, size, 0x180E);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x180E);
    spr.drawLine(x, y - size/2, x, y + size/2, 0x180E);
}

void drawHumilitySymbol(int x, int y, int size) {
    // Bowed head or kneeling figure - using simpler shape since drawArc needs more parameters
    spr.drawCircle(x, y, size/2, 0x780F);
    spr.drawLine(x, y, x, y + size/2, 0x780F);
}

void RoverViewManager::drawChakras() {
    LOG_SCOPE("Drawing chakras view");
    
    // Get current day of week (0 = Sunday)
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayIndex = timeInfo->tm_wday;
    
    const ChakraInfo& todayChakra = CHAKRA_DATA[dayIndex];
    
    // Draw title in black with smaller font
    spr.setTextFont(2);  // Smaller font for title
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString(todayChakra.name, SCREEN_CENTER_X, 210);
    
    // Draw chakra symbol
    todayChakra.drawSymbol(SCREEN_CENTER_X, 240, 40);
    
    // Draw attributes in black
    spr.setTextFont(2);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    
    // Split attributes into multiple lines if needed
    String attributes = todayChakra.attributes;
    int y = 270;
    int startPos = 0;
    int lastSpace = -1;
    
    for(int i = 0; i < attributes.length(); i++) {
        if(attributes[i] == ' ') {
            lastSpace = i;
        }
        
        // Check if we need to wrap (more than 25 chars or end of string)
        if(i - startPos > 25 || i == attributes.length() - 1) {
            String currentLine;
            if(i == attributes.length() - 1) {
                // Last line includes the final character
                currentLine = attributes.substring(startPos);
            } else {
                // Use last space for clean word break
                currentLine = attributes.substring(startPos, lastSpace);
                i = lastSpace;
            }
            spr.drawString(currentLine, SCREEN_CENTER_X, y);
            y += 20;
            startPos = lastSpace + 1;
        }
    }
}

void RoverViewManager::drawVirtues() {
    LOG_SCOPE("Drawing virtues view");
    
    // Get current day of week (0 = Sunday)
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayIndex = timeInfo->tm_wday;
    
    const VirtueInfo& todayVirtue = VIRTUE_DATA[dayIndex];
    
    // Draw virtue title in black
    spr.setTextFont(2);  // Smaller font
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString(todayVirtue.virtue, SCREEN_CENTER_X, 210);  // Moved down 5 pixels
    
    // Draw virtue symbol
    todayVirtue.drawSymbol(SCREEN_CENTER_X, 240, 40);  // Moved down 5 pixels
    
    // Draw description in black
    spr.setTextFont(2);
    spr.setTextColor(TFT_BLACK, FRAME_COLOR);
    spr.drawString(todayVirtue.description, SCREEN_CENTER_X, 270);  // Moved down 5 pixels
}

void RoverViewManager::drawNextMeal() {
    LOG_SCOPE("Drawing recipe view");
    
    // Draw recipe title
    spr.setTextFont(4);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("Garlic Pasta", SCREEN_CENTER_X, 200);
    
    // Draw ingredients list
    spr.setTextFont(2);
    spr.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    
    const char* ingredients[] = {
        "8 oz Spaghetti",
        "4 Garlic cloves",
        "3 tbsp Olive oil",
        "Salt & Pepper",
    };
    
    int y = 230;
    for(int i = 0; i < 5; i++) {
        spr.drawString(ingredients[i], SCREEN_CENTER_X, y);
        y += 20;
    }
}

// Implement all the chakra drawing functions
void drawRootChakra(int x, int y, int size) {
    // Basic square with downward triangle for root chakra
    spr.drawRect(x - size/2, y - size/2, size, size, TFT_RED);
    spr.fillTriangle(x, y + size/2, x - size/2, y - size/2, x + size/2, y - size/2, TFT_RED);
}

void drawSacralChakra(int x, int y, int size) {
    // Crescent moon shape for sacral chakra
    spr.drawCircle(x, y, size/2, 0xFDA0);
    spr.drawCircle(x + size/4, y, size/2, 0xFDA0);
}

void drawSolarChakra(int x, int y, int size) {
    // Sun-like pattern for solar plexus
    spr.drawCircle(x, y, size/2, 0xFFE0);
    for(int i = 0; i < 8; i++) {
        float angle = i * PI / 4;
        int x1 = x + cos(angle) * size/2;
        int y1 = y + sin(angle) * size/2;
        spr.drawLine(x, y, x1, y1, 0xFFE0);
    }
}

void drawHeartChakra(int x, int y, int size) {
    // Heart shape for heart chakra
    spr.fillCircle(x - size/4, y, size/4, 0x07E0);
    spr.fillCircle(x + size/4, y, size/4, 0x07E0);
    spr.fillTriangle(x - size/2, y, x + size/2, y, x, y + size/2, 0x07E0);
}

void drawThroatChakra(int x, int y, int size) {
    // Circle with wings for throat chakra
    spr.drawCircle(x, y, size/3, 0x001F);
    spr.drawLine(x - size/2, y, x + size/2, y, 0x001F);
    spr.drawLine(x - size/2, y - size/4, x + size/2, y - size/4, 0x001F);
}

void drawThirdEyeChakra(int x, int y, int size) {
    // Eye shape for third eye chakra
    spr.drawEllipse(x, y, size/2, size/3, 0x180E);
    spr.fillCircle(x, y, size/6, 0x180E);
}

void drawCrownChakra(int x, int y, int size) {
    // Crown-like pattern
    for(int i = 0; i < 7; i++) {
        int x1 = x - size/2 + (i * size/6);
        spr.drawLine(x1, y + size/2, x1, y - size/2, 0x780F);
    }
    spr.drawLine(x - size/2, y - size/2, x + size/2, y - size/2, 0x780F);
}   
