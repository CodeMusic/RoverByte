#ifndef FASTLED_HARDWARE_H
#define FASTLED_HARDWARE_H

#include <FastLED.h>
#include "../MotorCortex/PinDefinitions.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"
namespace VisualCortex 
{
    
    using namespace CorpusCallosum;

     
    namespace MC = MotorCortex;
    
    /**
     * @brief Hardware-specific LED configuration parameters
     * 
     * Defines physical characteristics and capabilities of the LED system
     */
    namespace FastLEDHardware 
    {
        using namespace CorpusCallosum;
        
        constexpr uint8_t DATA_PIN = VP::WS2812_DATA_PIN;
        constexpr uint8_t NUM_LEDS = VP::WS2812_NUM_LEDS;
        constexpr EOrder COLOR_ORDER = VP::WS2812_COLOR_ORDER;
        
        // LED Performance Parameters
        constexpr uint8_t MAX_BRIGHTNESS = 255;
        constexpr uint8_t DEFAULT_FPS = 60;
        constexpr auto COLOR_CORRECTION = TypicalLEDStrip;
        #define LED_TYPE WS2812B
    }
}

#endif // FASTLED_HARDWARE_H 