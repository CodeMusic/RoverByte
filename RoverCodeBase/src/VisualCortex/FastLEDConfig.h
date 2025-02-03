/**
 * @brief FastLED Hardware Configuration
 * 
 * Centralizes all FastLED-specific ESP32 hardware settings.
 * Must be included before FastLED.h in the main sketch.
 */

#ifndef FASTLED_CONFIG_H
#define FASTLED_CONFIG_H

// FastLED ESP32 Hardware Configuration
#define FASTLED_ESP32_RMT_DRIVER
#define FASTLED_RMT_MAX_CHANNELS 2
#define FASTLED_ESP32_FLASH_LOCK 1
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>
#include "FastLEDHardware.h"
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"

namespace VisualCortex 
{
    namespace PC = PrefrontalCortex;
    namespace FastLEDConfig 
    {
        using namespace FastLEDHardware;
        using namespace CorpusCallosum;
    }

    using AnimationTiming = PC::VisualTypes::AnimationPerception;
    using PatternConfig = PC::VisualTypes::PatternPerception;
    using BootConfig = PC::VisualTypes::BootPerception;
}

#endif // FASTLED_CONFIG_H 