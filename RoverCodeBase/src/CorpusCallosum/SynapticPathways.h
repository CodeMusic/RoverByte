/**
 * @brief SynapticPathways serves as the central nervous system for cross-cortex communication
 * 
 * This header establishes the core neural architecture of the system by:
 * - Defining cortex namespaces that represent specialized brain regions
 * - Establishing synaptic pathways (namespace aliases) for efficient signal routing
 * - Importing core cognitive types from the PrefrontalCortex
 * - Managing cross-cortex type systems and perceptions
 * 
 * The architecture mirrors biological neural organization:
 * - PrefrontalCortex: Executive function, decision making, and core types
 * - VisualCortex: Visual processing and display management
 * - AuditoryCortex: Sound processing and generation
 * - SomatosensoryCortex: Touch and UI interaction
 * - PsychicCortex: External communication (WiFi, IR, NFC)
 * - MotorCortex: Hardware control and pin definitions
 * - GameCortex: Entertainment and engagement systems
 */

#ifndef SYNAPTIC_PATHWAYS_H
#define SYNAPTIC_PATHWAYS_H

// Core system includes
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../MotorCortex/PinDefinitions.h"

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
    using PrefrontalCortex::VisualTypes::VisualPattern;
    using PrefrontalCortex::VisualTypes::VisualMessage;
}

namespace SomatosensoryCortex 
{
    class UIManager;
    class MenuManager;
}

namespace AuditoryCortex 
{
    class SoundFxManager;
    class PitchPerception;
    using PrefrontalCortex::AudioTypes::NoteInfo;
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
    // Cortex namespace aliases for neural pathway optimization
    namespace PC = PrefrontalCortex;
    namespace VC = VisualCortex;
    namespace SC = SomatosensoryCortex;
    namespace AC = AuditoryCortex;
    namespace PSY = PsychicCortex;
    namespace GC = GameCortex;
    namespace MC = MotorCortex;

    // Import core cognitive type systems from PrefrontalCortex
    using namespace PC::SystemTypes;     // Core system types
    using namespace PC::StorageTypes;    // Storage related types
    using namespace PC::SensorTypes;     // Sensor related types
    using namespace PC::ConfigTypes;     // Configuration types
    using namespace PC::PowerTypes;      // Power management types
    
    // Import specialized cognitive type systems
    using namespace PC::BehaviorTypes;   // Behavior related types
    using namespace PC::UITypes;         // UI related types
    using namespace PC::GameTypes;       // Game related types
    using namespace PC::CommTypes;       // Communication types
    using namespace PC::AudioTypes;      // Audio related types
    using namespace PC::VisualTypes;     // Visual related types
    using namespace PC::RoverTypes;      // Rover specific types
    
    // Import specialized neural subsystem types
    using namespace PC::ChakraTypes;     // Energy system types
    using namespace PC::VirtueTypes;     // Virtue system types
    using namespace PC::AuditoryTypes;   // Sound related types
    using namespace PC::PsychicTypes;    // ESP/remote sensing types

    // Instead, use the PrefrontalCortex types directly
    using PrefrontalCortex::VisualTypes::VisualPattern;
    using PrefrontalCortex::VisualTypes::VisualMessage;
    using PrefrontalCortex::AudioTypes::NoteInfo;

    // Create namespace aliases for neural pathways
    using VP = MC::PinDefinitions::VisualPathways;
    using AP = MC::PinDefinitions::AuditoryPathways;
    using TP = MC::PinDefinitions::TactilePathways;
    using CP = MC::PinDefinitions::CommunicationPathways;
    using SP = MC::PinDefinitions::StoragePathways;
    using LP = MC::PinDefinitions::LoRaPathways;
}

#endif // SYNAPTIC_PATHWAYS_H