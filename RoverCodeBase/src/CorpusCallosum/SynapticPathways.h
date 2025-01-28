#ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core system includes
#include <Arduino.h>
#include <FastLED.h>
#include <vector>

// Forward declare all cortex namespaces
namespace PrefrontalCortex 
{
    class Utilities;
    class SPIManager;
    class RoverBehaviorManager;
    class PowerManager;
    class SDManager;
    class WiFiManager;
}

namespace VisualCortex 
{
    class DisplayConfig;
    class LEDManager;
    class RoverViewManager;
    class RoverManager;
    class VisualSynesthesia;
    enum class VisualPattern;
    enum class VisualMessage;
}

namespace SomatosensoryCortex 
{
    class UIManager;
    class MenuManager;
}

namespace AuditoryCortex 
{
    class SoundFxManager;
    struct NoteInfo;
    class PitchPerception;
}

namespace PsychicCortex 
{
    class NFCManager;
    class IRManager;
    class WiFiManager;
}

namespace GameCortex 
{
    class SlotsManager;
    class AppManager;
}

namespace MotorCortex 
{
    class PinDefinitions;
}

// Create namespace aliases
namespace CorpusCallosum 
{
    namespace PC = PrefrontalCortex;
    namespace VC = VisualCortex;
    namespace SC = SomatosensoryCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    namespace GC = GameCortex;
    namespace MC = MotorCortex;
}

#endif