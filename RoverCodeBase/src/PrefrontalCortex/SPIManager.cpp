#include "SPIManager.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/utilities.h"

namespace PrefrontalCortex 
{
    namespace PC = PrefrontalCortex;  // Add namespace alias

    // Initialize state tracking
    bool SPIManager::initialized = false;

    bool SPIManager::isInitialized() 
    {
        return initialized;
    }

    void SPIManager::init() 
    {
        if (initialized) return;
        
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
        
        Utilities::LOG_DEBUG("SPI bus and chip selects initialized");
        initialized = true;
    }

    void SPIManager::selectDevice(uint8_t deviceCS) 
    {
        // Deselect all devices first
        deselectAll();
        
        // Select the requested device
        digitalWrite(deviceCS, LOW);
    }

    void SPIManager::deselectAll() 
    {
        digitalWrite(TFT_CS, HIGH);
        digitalWrite(BOARD_SD_CS, HIGH);
        digitalWrite(BOARD_LORA_CS, HIGH);
    }
}