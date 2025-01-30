#ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core system includes
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "../PrefrontalCortex/ProtoPerceptions.h"

// Forward declare all cortex namespaces
namespace PrefrontalCortex 
{
    class Utilities;
    class SPIManager;
    class RoverBehaviorManager;
    class PowerManager;
    class SDManager;
    class WiFiManager;

    // Import all type namespaces from ProtoPerceptions.h
    using namespace StorageTypes;
    using namespace SensorTypes;
    using namespace ConfigTypes;
    using namespace SystemTypes;
    using namespace BehaviorTypes;
    using namespace UITypes;
    using namespace GameTypes;
    using namespace CommTypes;
    using namespace AudioTypes;
    using namespace VisualTypes;
    using namespace RoverTypes;
    using namespace ChakraTypes;
    using namespace VirtueTypes;
    using namespace SomatosensoryTypes;
    using namespace AuditoryTypes;
    using namespace PsychicTypes;
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

    // Add these using declarations
    using namespace PC::StorageTypes;
    using namespace PC::SensorTypes;
    using namespace PC::ConfigTypes;
    using namespace PC::SystemTypes;
    using namespace PC::BehaviorTypes;
    using namespace PC::UITypes;
    using namespace PC::GameTypes;
    using namespace PC::CommTypes;
    using namespace PC::AudioTypes;
    using namespace PC::VisualTypes;
    using namespace PC::RoverTypes;
    using namespace PC::ChakraTypes;
    using namespace PC::VirtueTypes;
    using namespace PC::SomatosensoryTypes;
    using namespace PC::AuditoryTypes;
    using namespace PC::PsychicTypes;
}

#endif