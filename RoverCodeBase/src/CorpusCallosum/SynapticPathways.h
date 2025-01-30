#ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core system includes
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "../PrefrontalCortex/ProtoPerceptions.h"

// Forward declare all cortex namespaces and their components
namespace PrefrontalCortex 
{
    // Core management classes
    class Utilities;
    class SPIManager;
    class RoverBehaviorManager;
    class PowerManager;
    class SDManager;
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

// Create namespace aliases for easier access
namespace CorpusCallosum 
{
    // Cortex namespace aliases
    namespace PC = PrefrontalCortex;
    namespace VC = VisualCortex;
    namespace SC = SomatosensoryCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    namespace GC = GameCortex;
    namespace MC = MotorCortex;

    // Import core type systems from PrefrontalCortex
    using namespace PC::SystemTypes;     // Core system types
    using namespace PC::StorageTypes;    // Storage related types
    using namespace PC::SensorTypes;     // Sensor related types
    using namespace PC::ConfigTypes;     // Configuration types
    
    // Import specialized type systems as needed
    using namespace PC::BehaviorTypes;   // Behavior related types
    using namespace PC::UITypes;         // UI related types
    using namespace PC::GameTypes;       // Game related types
    using namespace PC::CommTypes;       // Communication types
    using namespace PC::AudioTypes;      // Audio related types
    using namespace PC::VisualTypes;     // Visual related types
    using namespace PC::RoverTypes;      // Rover specific types
    
    // Import specialized subsystem types
    using namespace PC::ChakraTypes;     // Energy system types
    using namespace PC::VirtueTypes;     // Virtue system types
    using namespace PC::SomatosensoryTypes;  // Touch/feel types
    using namespace PC::AuditoryTypes;   // Sound related types
    using namespace PC::PsychicTypes;    // ESP/remote sensing types
}

#endif // SYNAPTIC_PATHWAYS_H