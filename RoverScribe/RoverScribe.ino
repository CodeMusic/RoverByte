#include <Arduino.h>
#include "epd_driver.h"
#include "utilities.h"
#include "RoverDisplay.h"
#include "ButtonHandler.h"

uint8_t *framebuffer;
RoverDisplay roverDisplay;
ButtonHandler buttonHandler;

const char* tasks[] = {
    "1. ServiceCanada",
    "2. Call new Doctor",
    "3. Contact Therapist (re:trust)",
    "4. Finish RoverShow Demo 1",
    "5. Finish Cleaning Kitchen/Entrance",
    "6. Laundry",
    "7. Contact Osgoode"
};

void setup() {
    Serial.begin(115200);
    epd_init();

    // Initialize buttons
    buttonHandler.init();
    
    // Initialize framebuffer
    framebuffer = (uint8_t *)heap_caps_malloc(EPD_WIDTH * EPD_HEIGHT / 2, MALLOC_CAP_SPIRAM);
    if (!framebuffer) {
        Serial.println("alloc memory failed !!!");
        while (1);
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    
    // Initial draw
    updateDisplay();
}

void loop() {
    buttonHandler.handle();
    if (buttonHandler.wasPressed()) {
        updateDisplay();
    }
    delay(50);
}

void updateDisplay() {
    epd_poweron();
    epd_clear();
    
    // Draw Rover and tasks
    roverDisplay.drawRover(framebuffer, EPD_WIDTH/2, 100);
    roverDisplay.drawTasks(framebuffer, tasks, sizeof(tasks)/sizeof(tasks[0]));
    
    // Update display
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    epd_poweroff();
}