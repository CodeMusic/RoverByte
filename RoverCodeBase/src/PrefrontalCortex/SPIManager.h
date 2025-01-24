#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"

class SPIManager {
public:
    static void initSPI() {
        // Initialize all CS pins as outputs and set them HIGH (disabled)
        pinMode(TFT_CS, OUTPUT);
        pinMode(BOARD_SD_CS, OUTPUT);
        pinMode(BOARD_LORA_CS, OUTPUT);
        
        // Ensure all devices are deselected
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
        
        // Configure SPI bus
        SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI);
        SPI.setFrequency(20000000); // 20MHz - adjust if needed
        
        LOG_DEBUG("SPI bus and chip selects initialized");
    }
    
    static void selectDevice(uint8_t deviceCS) {
        // Deselect all devices first
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
        
        // Select the requested device
        digitalWrite(deviceCS, LOW);
    }
    
    static void deselectAll() {
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
    }
}; 