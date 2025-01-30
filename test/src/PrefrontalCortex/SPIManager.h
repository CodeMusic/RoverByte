#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "../PrefrontalCortex/utilities.h"
#include "../MotorCortex/PinDefinitions.h"
#include "../CorpusCallosum/SynapticPathways.h"

using namespace CorpusCallosum;

namespace PrefrontalCortex 
{

    class SPIManager {
    public:
        // Check if SPIManager is initialized
        static bool isInitialized();

        // Initialize SPI and CS pins
        static void init();

        // Select a specific SPI device
        static void selectDevice(uint8_t deviceCS);

        // Deselect all SPI devices
        static void deselectAll();

    private:
        static bool initialized;
    };

}