#pragma once
#include "epd_driver.h"

class RoverDisplay {
public:
    RoverDisplay() : x_cursor(50), y_cursor(200) {}  // Initialize cursor positions
    void drawRover(uint8_t* framebuffer, int x, int y);
    void drawTasks(uint8_t* framebuffer, const char* tasks[], int taskCount);
private:
    int x_cursor;
    int y_cursor;
}; 