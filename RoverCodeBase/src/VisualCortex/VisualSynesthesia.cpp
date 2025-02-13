/**
 * @brief Implementation of cross-modal sensory integration system
 * 
 * Implements core synesthetic processing:
 * - Color-emotion mapping algorithms
 * - Audio-visual translation
 * - Temporal-chromatic correlation
 * - Energy state visualization
 * - Multi-sensory binding
 * 
 * Key processing pathways:
 * - Frequency to color conversion
 * - Emotional state coloring
 * - Circadian rhythm visualization
 * - Energy level indication
 * - Cross-modal feedback loops
 */

#include "../VisualCortex/VisualSynesthesia.h"
#include "../AuditoryCortex/SoundFxManager.h"
#include "../VisualCortex/LEDManager.h"
#include <FastLED.h>
#include "../PrefrontalCortex/Utilities.h"

namespace VisualCortex 
{
    namespace AC = AuditoryCortex;
    namespace PC = PrefrontalCortex;
    using AC::PitchPerception;
    using namespace PC::ColorPerceptionTypes;

    CRGB VisualSynesthesia::getBase8Color(uint8_t cognitiveValue) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getBase8Color(uint8_t)", String(cognitiveValue));
        return PC::ColorPerceptionTypes::BASE_8_COLORS[cognitiveValue % 8];
    }

    CRGB VisualSynesthesia::getDayColor(uint8_t circadianIndex) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getDayColor(uint8_t)", String(circadianIndex));
        return PC::ColorPerceptionTypes::DAY_COLORS[(circadianIndex - 1) % 7];
    }

    void VisualSynesthesia::getMonthColors(uint8_t temporalIndex, CRGB& primaryPerception, CRGB& secondaryPerception) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getMonthColors(uint8_t, CRGB&, CRGB&)", String(temporalIndex));
        temporalIndex = (temporalIndex - 1) % 12;
        primaryPerception = PC::ColorPerceptionTypes::MONTH_COLORS[temporalIndex][0];
        secondaryPerception = PC::ColorPerceptionTypes::MONTH_COLORS[temporalIndex][1];
    }

    void VisualSynesthesia::getHourColors(uint8_t circadianHour, CRGB& primaryPerception, CRGB& secondaryPerception) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getHourColors(uint8_t, CRGB&, CRGB&)", String(circadianHour));
        circadianHour = (circadianHour - 1) % 12;
        primaryPerception = PC::ColorPerceptionTypes::CHROMATIC_COLORS[circadianHour][0];
        secondaryPerception = PC::ColorPerceptionTypes::CHROMATIC_COLORS[circadianHour][1];
    }

    CRGB VisualSynesthesia::getBatteryColor(uint8_t energyLevel) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getBatteryColor(uint8_t)", String(energyLevel));
        if (energyLevel > 75) 
        {
            return CRGB::Green;
        } 
        else if (energyLevel > 50) 
        {
            return CRGB::Yellow;
        } 
        else if (energyLevel > 25) 
        {
            return CRGB(255, 140, 0);  // Orange
        } 
        else 
        {
            return CRGB::Red;
        }
    }

    CRGB VisualSynesthesia::getColorForFrequency(uint16_t freq) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getColorForFrequency(uint16_t)", String(freq));
        if (freq >= PitchPerception::NOTE_C5) return PC::ColorPerceptionTypes::BASE_8_COLORS[1];      // Red
        else if (freq >= PitchPerception::NOTE_B4) return PC::ColorPerceptionTypes::BASE_8_COLORS[7]; // Violet
        else if (freq >= PitchPerception::NOTE_A4) return PC::ColorPerceptionTypes::BASE_8_COLORS[6]; // Indigo
        else if (freq >= PitchPerception::NOTE_G4) return PC::ColorPerceptionTypes::BASE_8_COLORS[5]; // Blue
        else if (freq >= PitchPerception::NOTE_F4) return PC::ColorPerceptionTypes::BASE_8_COLORS[4]; // Green
        else if (freq >= PitchPerception::NOTE_E4) return PC::ColorPerceptionTypes::BASE_8_COLORS[3]; // Yellow
        else if (freq >= PitchPerception::NOTE_D4) return PC::ColorPerceptionTypes::BASE_8_COLORS[2]; // Orange
        else return PC::ColorPerceptionTypes::BASE_8_COLORS[1];                                       // Red
    }

    CRGB VisualSynesthesia::blendNoteColors(const NoteInfo& musicalPerception) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::blendNoteColors(NoteInfo)");
        if (musicalPerception.isSharp) 
        {
            // For sharp notes, blend the colors of the adjacent natural notes
            CRGB lowerColor = PC::ColorPerceptionTypes::CHROMATIC_COLORS[(int(musicalPerception.note) - 1 + 12) % 12][0];
            CRGB upperColor = PC::ColorPerceptionTypes::CHROMATIC_COLORS[(int(musicalPerception.note) + 1) % 12][0];
            return blend(lowerColor, upperColor, 128); // 50% blend
        } 
        else 
        {
            return PC::ColorPerceptionTypes::CHROMATIC_COLORS[int(musicalPerception.note)][0];
        }
    }

    void VisualSynesthesia::playVisualChord(uint16_t baseFreq, CRGB& rootPerception, CRGB& thirdPerception, CRGB& fifthPerception) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::playVisualChord(uint16_t, CRGB&, CRGB&, CRGB&)", 
            String(baseFreq));
        // Get root note information
        NoteInfo rootInfo = AC::PitchPerception::getNoteInfo(baseFreq);

        // Calculate major third (+4 semitones) and perfect fifth (+7 semitones)
        int thirdNoteIndex = (int(rootInfo.note) + 4) % 12;
        int thirdOctave = rootInfo.octave + ((int(rootInfo.note) + 4) >= 12 ? 1 : 0);
        int fifthNoteIndex = (int(rootInfo.note) + 7) % 12;
        int fifthOctave = rootInfo.octave + ((int(rootInfo.note) + 7) >= 12 ? 1 : 0);

        // Calculate frequencies for third and fifth
        uint16_t thirdFreq = AC::PitchPerception::getNoteFrequencies()[(thirdOctave - 1) * 12 + thirdNoteIndex];
        uint16_t fifthFreq = AC::PitchPerception::getNoteFrequencies()[(fifthOctave - 1) * 12 + fifthNoteIndex];

        // Get NoteInfo for third and fifth
        NoteInfo thirdInfo = AC::PitchPerception::getNoteInfo(thirdFreq);
        NoteInfo fifthInfo = AC::PitchPerception::getNoteInfo(fifthFreq);

        // Play the chord sequence with visual feedback
        AC::SoundFxManager::playTone(baseFreq, 200);
        LEDManager::displayNote(baseFreq, 0);
        LEDManager::displayNote(baseFreq, 1);
        
        AC::SoundFxManager::playTone(thirdFreq, 200);
        LEDManager::displayNote(thirdFreq, 2);
        LEDManager::displayNote(thirdFreq, 3);
        
        AC::SoundFxManager::playTone(fifthFreq, 200); 
        LEDManager::displayNote(fifthFreq, 3);
        LEDManager::displayNote(fifthFreq, 4);
        
        AC::SoundFxManager::playTone(thirdFreq, 200);
        LEDManager::displayNote(thirdFreq, 5);
        LEDManager::displayNote(thirdFreq, 6);
        
        AC::SoundFxManager::playTone(baseFreq, 200);
        LEDManager::displayNote(baseFreq, 7);

        // Get colors for the chord
        rootPerception = getColorForFrequency(AC::PitchPerception::getNoteFrequency(rootInfo));
        thirdPerception = getColorForFrequency(AC::PitchPerception::getNoteFrequency(thirdInfo));
        fifthPerception = getColorForFrequency(AC::PitchPerception::getNoteFrequency(fifthInfo));
    }

    uint16_t VisualSynesthesia::convertToRGB565(CRGB neuralColor) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::convertToRGB565(CRGB)");
        return ((neuralColor.r & 0xF8) << 8) | ((neuralColor.g & 0xFC) << 3) | (neuralColor.b >> 3);
    }

    void VisualSynesthesia::playNFCCardData(const char* cardData) {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::playNFCCardData(const char*)", cardData);
        for (int i = 0; i < strlen(cardData); i++) {
            // Map each character to a frequency
            uint16_t frequency = map(cardData[i], 32, 126, 200, 2000); // Map printable ASCII to a frequency range
            AC::SoundFxManager::playTone(frequency, 200); // Play each note for 200 ms
            delay(250); // Delay between notes
        }
    }

    ChromaticContext VisualSynesthesia::getChromaticContext(uint16_t frequency) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getChromaticContext(uint16_t)", String(frequency));
        ChromaticContext context;
        context.primary = getColorForFrequency(frequency);
        context.secondary = getColorForFrequency(frequency + 100); // Slight shift for secondary
        context.intensity = map(frequency, 200, 2000, 0, 255);
        return context;
    }

    EmotionalColor VisualSynesthesia::getEmotionalColor(PC::RoverTypes::Expression emotion) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::getEmotionalColor(Expression)", String(static_cast<int>(emotion)));
        EmotionalColor color;
        color.emotion = emotion;
        
        switch(emotion) 
        {
            case PC::RoverTypes::Expression::HAPPY:
                color.primaryColor = CRGB::Yellow;
                color.accentColor = CRGB::Orange;
                break;
            case PC::RoverTypes::Expression::LOOKING_UP:
                color.primaryColor = CRGB::Blue;
                color.accentColor = CRGB::Cyan;
                break;
            // Add other expressions...
            default:
                color.primaryColor = CRGB::Green;
                color.accentColor = CRGB::Blue;
        }
        return color;
    }

    void VisualSynesthesia::playVisualChord(uint16_t fundamentalFreq, 
                                           ChromaticContext& rootContext,
                                           ChromaticContext& thirdContext,
                                           ChromaticContext& fifthContext) 
    {
        PC::Utilities::LOG_SCOPE("VisualCortex::VisualSynesthesia::playVisualChord(uint16_t, ChromaticContext&, ChromaticContext&, ChromaticContext&)", 
            String(fundamentalFreq));
        // Calculate frequencies
        uint16_t thirdFreq = fundamentalFreq * 5 / 4;  // Major third
        uint16_t fifthFreq = fundamentalFreq * 3 / 2;  // Perfect fifth
        
        // Get contexts
        rootContext = getChromaticContext(fundamentalFreq);
        thirdContext = getChromaticContext(thirdFreq);
        fifthContext = getChromaticContext(fifthFreq);
        
        // Play the frequencies with visual feedbacka
        LEDManager::displayChromatic(rootContext);
        AC::SoundFxManager::playTone(fundamentalFreq, 200);
        delay(250);
        
        LEDManager::displayChromatic(thirdContext);
        AC::SoundFxManager::playTone(thirdFreq, 200);
        delay(250);
        
        LEDManager::displayChromatic(fifthContext);
        AC::SoundFxManager::playTone(fifthFreq, 200);
        delay(250);
    }
}