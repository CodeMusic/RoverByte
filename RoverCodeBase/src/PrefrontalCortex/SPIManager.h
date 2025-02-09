#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "PrefrontalCortex/Utilities.h"
#include "PrefrontalCortex/ProtoPerceptions.h"
#include "MotorCortex/PinDefinitions.h"
#include "CorpusCallosum/SynapticPathways.h"

namespace PrefrontalCortex 
{
    using namespace CorpusCallosum;

    /**
     * @brief Manages SPI bus communication and device selection
     * 
     * Controls:
     * - SPI bus initialization and configuration
     * - Device chip select management
     * - Multi-device coordination
     * - Safe device switching
     * - High-speed data transfer
     */
    class SPIManager 
    {
    public:
        // Check if SPI bus is ready
        static bool isInitialized();

        // Initialize SPI bus and chip selects
        static void init();

        // Select specific device for communication
        static void selectDevice(uint8_t deviceCS);

        // Deselect all devices
        static void deselectAll();

    private:
        // Track initialization state
        static bool initialized;
    };
}