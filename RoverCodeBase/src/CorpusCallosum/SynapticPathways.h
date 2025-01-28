#ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core includes first
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SPIFFS.h>
#include "driver/i2s.h"

// Forward declarations of all cortex namespaces
namespace VisualCortex {}
namespace PrefrontalCortex {}
namespace AuditoryCortex {}
namespace PsychicCortex {}
namespace MotorCortex {}
namespace SomatosensoryCortex {}
namespace GameCortex {}

// FastLED Configuration
#include "../VisualCortex/FastLEDConfig.h"

// Common Project Headers
#include "../PrefrontalCortex/Utilities.h"
#include "../VisualCortex/VisualSynesthesia.h"
#include "../AuditoryCortex/PitchPerception.h"

namespace CorpusCallosum 
{
    // Cortex namespace aliases
    namespace VC = VisualCortex;
    namespace PC = PrefrontalCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    namespace MC = MotorCortex;
    namespace SC = SomatosensoryCortex;
    namespace GC = GameCortex;
}

#endif