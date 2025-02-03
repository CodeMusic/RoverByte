/**
 * @file SoundFxManager.cpp
 * @brief Implementation of the SoundFxManager class for audio output and effects
 * 
 * Handles the implementation of audio playback, recording, and sound effect generation.
 * Includes initialization of audio hardware and management of audio resources.
 */

#include "../PrefrontalCortex/ProtoPerceptions.h"
#include "../CorpusCallosum/SynapticPathways.h"
#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../VisualCortex/LEDManager.h"
#include <vector>
#include "../VisualCortex/VisualSynesthesia.h"
#include "../PrefrontalCortex/Utilities.h"
#include "../MotorCortex/PinDefinitions.h"

namespace AuditoryCortex
{
    using namespace CorpusCallosum;
    using VC::RoverManager;
    using VC::LEDManager;
    using VC::VisualSynesthesia;
    using PC::Utilities;
    using MC::PinDefinitions;
    using PC::AudioTypes::TunesTypes;
    using PC::AudioTypes::Tune;
    using PC::AudioTypes::NoteInfo;
    using PC::AudioTypes::TimeSignature;

    // Initialize static members with meaningful defaults
    bool SoundFxManager::_isInitialized = false;
    bool SoundFxManager::isRecording = false;
    bool SoundFxManager::isPlayingSound = false;
    int SoundFxManager::volume = 42;  // Default volume level
    
    // Initialize static members
    int SoundFxManager::currentNote = 0;
    unsigned long SoundFxManager::lastNoteTime = 0;
    bool SoundFxManager::m_isTunePlaying = false;
    Audio SoundFxManager::audio;
    const char* SoundFxManager::RECORD_FILENAME = "/sdcard/temp_record.wav";
    File SoundFxManager::recordFile;

    PC::AudioTypes::TunesTypes SoundFxManager::selectedSong = PC::AudioTypes::TunesTypes::ROVERBYTE_JINGLE;
    PC::AudioTypes::Tune SoundFxManager::activeTune;

    void SoundFxManager::playTone(int frequency, int duration, int volume) 
    {
        if (frequency <= 0) return;

        const int TONE_PWM_CHANNEL = 0;  // Define PWM channel
        ledcSetup(TONE_PWM_CHANNEL, frequency, 8);  // 8-bit resolution
        ledcWrite(TONE_PWM_CHANNEL, volume);
        
        if (duration > 0) 
        {
            delay(duration);
            ledcWrite(TONE_PWM_CHANNEL, 0);
        }
    }

    void SoundFxManager::playTune(PC::AudioTypes::TunesTypes type) 
    {
        selectedSong = type;
        switch (type) 
        {
            case PC::AudioTypes::TunesTypes::ROVERBYTE_JINGLE:
            case PC::AudioTypes::TunesTypes::JINGLE_BELLS:
            case PC::AudioTypes::TunesTypes::AULD_LANG_SYNE:
            case PC::AudioTypes::TunesTypes::LOVE_SONG:
            case PC::AudioTypes::TunesTypes::HAPPY_BIRTHDAY:
            case PC::AudioTypes::TunesTypes::EASTER_SONG:
            case PC::AudioTypes::TunesTypes::MOTHERS_SONG:
            case PC::AudioTypes::TunesTypes::FATHERS_SONG:
            case PC::AudioTypes::TunesTypes::CANADA_SONG:
            case PC::AudioTypes::TunesTypes::USA_SONG:
            case PC::AudioTypes::TunesTypes::CIVIC_SONG:
            case PC::AudioTypes::TunesTypes::WAKE_ME_UP_WHEN_SEPTEMBER_ENDS:
            case PC::AudioTypes::TunesTypes::HALLOWEEN_SONG:
            case PC::AudioTypes::TunesTypes::THANKSGIVING_SONG:
            case PC::AudioTypes::TunesTypes::CHRISTMAS_SONG:
                startTune();  // This will load and play the selected tune
                break;

            default:
                Utilities::LOG_WARNING("Unknown tune type requested");
                break;
        }
    }

    void SoundFxManager::startTune() 
    {
        currentNote = 0;
        m_isTunePlaying = true;
        lastNoteTime = 0;
        activeTune = Tunes::getTune(selectedSong);
    }

    void SoundFxManager::updateTune() 
    {
        if (!m_isTunePlaying || currentNote >= activeTune.notes.size()) 
        {
            m_isTunePlaying = false;
            return;
        }
        
        unsigned long currentTime = millis();
        const PC::AudioTypes::NoteInfo& note = activeTune.notes[currentNote];
        int noteDuration = PitchPerception::getNoteDuration(note.type, activeTune.timeSignature);
        
        if (currentTime - lastNoteTime >= noteDuration) 
        {
            if (currentNote < activeTune.notes.size()) 
            {
                uint16_t frequency = PitchPerception::getNoteFrequency(note);
                playTone(frequency, noteDuration, volume);
                
                // Update LED visualization
                for (int j = 0; j < MC::PinDefinitions::VisualPathways::WS2812_NUM_LEDS; j++) 
                {
                    if (bitRead(activeTune.ledAnimation[currentNote], j)) 
                    {
                        CRGB color = VisualSynesthesia::getNoteColorBlended(note);
                        LEDManager::setLED(j, color);
                    }
                }
                LEDManager::showLEDs();
                
                lastNoteTime = currentTime;
                currentNote++;
            } 
            else 
            {
                m_isTunePlaying = false;
            }
        }
    }

    void SoundFxManager::playSuccessSound() {
        playTone(PitchPerception::NOTE_C5, 100);
        delay(50);
        playTone(PitchPerception::NOTE_E5, 100);
        delay(50);
        playTone(PitchPerception::NOTE_G5, 200);
    }

    void SoundFxManager::playRotaryPressSound(int mode)  // 0=Full, 1=Week, 2=Timer
    {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        int dayOfWeek = timeInfo->tm_wday;  // 0-6 (Sunday-Saturday)
        uint16_t baseNote = PitchPerception::getDayBaseNote(mode == 1);

        switch(mode) 
        {
            case 0: 
                playTone(baseNote, 100);
                break;
            case 1:  
                playTone(baseNote, 100);  
                break;
            case 2:  // Timer mode - octave up + fifth
                playTone(baseNote * 2, 100);
                break;
        }
    }

    void SoundFxManager::playRotaryTurnSound(bool clockwise) {
        if (clockwise) {
            playTone(PitchPerception::getDayBaseNote4(), 50);
            playTone(PitchPerception::getDayBaseNote5(), 50);
        } else {
            playTone(PitchPerception::getDayBaseNote5(), 50);
            playTone(PitchPerception::getDayBaseNote4(), 50);
        }
    }

    void SoundFxManager::playSideButtonSound(bool start) {
        if (start) {
            playTone(PitchPerception::getDayBaseNote4(), 50);
            playTone(PitchPerception::getDayBaseNote4(), 50);
        } else {
            playTone(PitchPerception::getDayBaseNote5(), 100);
            int baseNote = PitchPerception::getDayBaseNote5();
            playTone(PitchPerception::getNoteMinus2(baseNote), 100);
        }
    }

    void SoundFxManager::playErrorSound(ErrorSoundType type) 
    {
        switch(type) 
        {
            case ErrorSoundType::RECORDING:
                SoundFxManager::playTone(PitchPerception::NOTE_B5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 400);
                break;
                
            case ErrorSoundType::STORAGE:
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_G4, 400);
                break;
                
            case ErrorSoundType::PLAYBACK:
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
                delay(100);
                SoundFxManager::playTone(PitchPerception::NOTE_D4, 400);
                break;
        }
    }

    // Add audio callback
    void SoundFxManager::audio_eof_mp3(const char *info) {
        Serial.printf("Audio playback finished: %s\n", info);
        // Delete temporary recording after playback
        if (!SD.remove(RECORD_FILENAME)) {
            Serial.println("Failed to delete temporary recording file");
            playErrorSound(ErrorSoundType::STORAGE);
        }
        isPlayingSound = false;
    }


    void SoundFxManager::startRecording() {
        if (!PC::SDManager::isInitialized()) return;
        if (isRecording) return;
        
        Serial.println("=== Starting Recording ===");
        
        // Initialize microphone
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
            .sample_rate = EXAMPLE_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 4,
            .dma_buf_len = 64,
            .use_apll = false,
        };

        i2s_pin_config_t pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_PIN_NO_CHANGE,
            .ws_io_num = BOARD_MIC_CLK,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = BOARD_MIC_DATA,
        };

        if (i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL) != ESP_OK) {
            Serial.println("ERROR: Failed to install I2S driver");
            playErrorSound(ErrorSoundType::RECORDING);
            RoverManager::setEarsPerked(false);
            return;
        }
        
        if (i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config) != ESP_OK) {
            Serial.println("ERROR: Failed to set I2S pins");
            i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
            playErrorSound(ErrorSoundType::RECORDING);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Create new WAV file
        recordFile = SD.open(RECORD_FILENAME, FILE_WRITE);
        if (!recordFile) {
            Serial.println("ERROR: Failed to open file for recording");
            i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
            playErrorSound(ErrorSoundType::RECORDING);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Reserve space for the header
        for (int i = 0; i < WAVE_HEADER_SIZE; i++) {
            recordFile.write(0);
        }

        isRecording = true;
        Serial.println("Recording started!");
    }

    void SoundFxManager::stopRecording() 
    {
        if (!isRecording) return;

        Serial.println("=== Stopping Recording ===");
        isRecording = false;
        
        // Memory-safe header generation
        uint32_t fileSize = recordFile.size() - WAVE_HEADER_SIZE;
        char wavHeader[WAVE_HEADER_SIZE];
        generate_wav_header(wavHeader, fileSize, EXAMPLE_SAMPLE_RATE);
        
        // Error handling with cognitive state tracking
        bool headerWriteSuccess = true;
        
        if (!recordFile.seek(0)) 
        {
            Serial.println("ERROR: Failed to seek in file");
            headerWriteSuccess = false;
        }
        else if (recordFile.write((uint8_t *)wavHeader, WAVE_HEADER_SIZE) != WAVE_HEADER_SIZE) 
        {
            Serial.println("ERROR: Failed to write WAV header");
            headerWriteSuccess = false;
        }
        
        // Cleanup resources
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        
        if (!headerWriteSuccess) 
        {
            playErrorSound(ErrorSoundType::RECORDING);
            RoverManager::setEarsPerked(false);
            return;
        }

        // Attempt playback
        audio.setVolume(volume);
        if (!SD.exists(RECORD_FILENAME) || !audio.connecttoFS(SD, RECORD_FILENAME)) 
        {
            Serial.println("ERROR: Playback failed");
            playErrorSound(ErrorSoundType::PLAYBACK);
            RoverManager::setEarsPerked(false);
            return;
        }

        RoverManager::setEarsPerked(false);
    }


    void SoundFxManager::init_microphone() {
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
            .sample_rate = EXAMPLE_SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
            .dma_buf_count = 8,
            .dma_buf_len = 200,
            .use_apll = 0
        };

        i2s_pin_config_t pin_config = {
            .mck_io_num = I2S_PIN_NO_CHANGE,
            .bck_io_num = I2S_PIN_NO_CHANGE,
            .ws_io_num = BOARD_MIC_CLK,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = BOARD_MIC_DATA
        };

        ESP_ERROR_CHECK(i2s_driver_install((i2s_port_t)EXAMPLE_I2S_CH, &i2s_config, 0, NULL));
        ESP_ERROR_CHECK(i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config));
        ESP_ERROR_CHECK(i2s_set_clk((i2s_port_t)EXAMPLE_I2S_CH, EXAMPLE_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO));
    }


    void SoundFxManager::generate_wav_header(char* wav_header, uint32_t wav_size, uint32_t sample_rate)
    {
        // Cognitive mapping of WAV format structure
        const WavHeaderInfo headerInfo = 
        {
            .fileSize = wav_size + WAVE_HEADER_SIZE - 8,
            .byteRate = BYTE_RATE,
            .sampleRate = sample_rate,
            .numChannels = 1,  // Mono recording
            .bitsPerSample = 16
        };

        // Memory-safe header construction using array bounds
        const size_t headerSize = WAVE_HEADER_SIZE;
        
        // Validate output buffer
        if (!wav_header) return;

        // Clear header memory first
        memset(wav_header, 0, headerSize);

        // Construct header in a memory-safe way
        const char header[WAVE_HEADER_SIZE] = 
        {
            // RIFF chunk descriptor
            'R', 'I', 'F', 'F',
            static_cast<char>(headerInfo.fileSize & 0xFF),
            static_cast<char>((headerInfo.fileSize >> 8) & 0xFF),
            static_cast<char>((headerInfo.fileSize >> 16) & 0xFF), 
            static_cast<char>((headerInfo.fileSize >> 24) & 0xFF),
            
            // WAVE chunk
            'W', 'A', 'V', 'E',
            
            // Format subchunk
            'f', 'm', 't', ' ',
            0x10, 0x00, 0x00, 0x00,  // Subchunk1Size (16 for PCM)
            0x01, 0x00,              // AudioFormat (1 for PCM)
            static_cast<char>(headerInfo.numChannels & 0xFF),
            0x00,
            
            // Sample rate
            static_cast<char>(headerInfo.sampleRate & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 8) & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 16) & 0xFF),
            static_cast<char>((headerInfo.sampleRate >> 24) & 0xFF),
            
            // Byte rate
            static_cast<char>(headerInfo.byteRate & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 8) & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 16) & 0xFF),
            static_cast<char>((headerInfo.byteRate >> 24) & 0xFF)
        };

        // Copy constructed header to output buffer
        memcpy(wav_header, header, headerSize);
    }

    void SoundFxManager::init() {
        if (_isInitialized) return;
        // Initialize audio hardware
        audio.setPinout(BOARD_VOICE_BCLK, BOARD_VOICE_LRCLK, BOARD_VOICE_DIN);
        audio.setVolume(volume);  // Set a reasonable volume level
        
        // Initialize SPIFFS for sound files
        if (!SPIFFS.begin(true)) {
            Utilities::LOG_ERROR("Failed to initialize SPIFFS");
            return;
        }
        
        // Initialize I2S
        i2s_config_t i2s_config = 
        {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = 44100,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,  // Increased from 4 for better buffering
            .dma_buf_len = 64,   // Increased from 32 for better performance
            .use_apll = true     // Changed to true for better audio quality
        };
        
        if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) != ESP_OK) 
        {
            Utilities::LOG_ERROR("I2S driver installation failed");
            return;
        }

        Utilities::LOG_PROD("I2S driver installed successfully");
        playStartupSound();
        _isInitialized = true;
    }

    void SoundFxManager::adjustVolume(int amount) {
        volume += amount;
        
        // Cognitive boundary checks for volume limits
        if (volume < 0) 
        {
            volume = 91;  // Wrap around to max volume
        }
        if (volume > 100) 
        {
            volume = 0;   // Wrap around to mute
        }
        
        audio.setVolume(volume);
    }

  

    void SoundFxManager::playVoiceLine(const char* line, uint32_t cardId) {
        if (strcmp(line, "card_detected") == 0 && cardId != 0) {
            playCardMelody(cardId);
        }
        else if (strcmp(line, "waiting_for_card") == 0) {
            // Inquisitive searching tune
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_A5, 150, 2);
        }
        else if (strcmp(line, "scan_complete") == 0) {
            // Success tune
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 0);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 1);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 2);
            delay(30);
            SoundFxManager::playTone(PitchPerception::NOTE_C6, 200);
        }
        else if (strcmp(line, "scan_error") == 0) {
            // Error tune
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 200, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E4, 200, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C4, 300, 2);
        }
        else if (strcmp(line, "level_up") == 0) {
            // Mario-style level up fanfare
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 2);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 3);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C6, 150, 4);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_E6, 400);
        }
        else if (strcmp(line, "volume_up") == 0) {
            // Volume up tune
            SoundFxManager::playTone(PitchPerception::NOTE_C5, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E5, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 100, 2);
        }
        else if (strcmp(line, "volume_down") == 0) {
            // Volume down tune
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 100, 0);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_E4, 100, 1);
            delay(50);
            SoundFxManager::playTone(PitchPerception::NOTE_C4, 100, 2);
        }
        
    }

    void SoundFxManager::playMenuCloseSound() {
        // Play a descending tone sequence for closing
        playTone(PitchPerception::NOTE_C5, 100); // C5
        delay(50);
        playTone(PitchPerception::NOTE_B4, 100); // B4
        delay(50);
        playTone(PitchPerception::NOTE_A4, 100); // A4
        delay(50);
        playTone(PitchPerception::NOTE_G4, 100); // G4
    }

    void SoundFxManager::playMenuOpenSound() {
        // Play an ascending tone sequence for opening
        playTone(PitchPerception::NOTE_G4, 100); // G4
        delay(50);
        playTone(PitchPerception::NOTE_A4, 100); // A4
        delay(50);
        playTone(PitchPerception::NOTE_B4, 100); // B4
        delay(50);
        playTone(PitchPerception::NOTE_C5, 100); // C5
    }

    void SoundFxManager::playMenuSelectSound() {
        // Play a short, sharp tone with a slight variation
        playTone(PitchPerception::NOTE_E5, 100); // E5 for selection
        delay(50);
        playTone(PitchPerception::NOTE_C5, 100); // C5
        delay(50);
        playTone(PitchPerception::NOTE_E5, 100); // E5 again
        delay(50);
        playTone(PitchPerception::NOTE_G5, 100); // G5 for a higher note
    }

    void SoundFxManager::playCardMelody(uint32_t cardId) 
    {
        // Generate melody from card ID with harmonic relationships
        const uint8_t MELODY_LENGTH = 4;
        uint8_t notes[MELODY_LENGTH];
        
        // Extract meaningful patterns from card ID
        notes[0] = ((cardId >> 24) & 0xFF) % 12;  // Root note (first octave)
        notes[1] = ((cardId >> 16) & 0xFF) % 12 + 12;  // Harmony note (second octave)
        notes[2] = ((cardId >> 8) & 0xFF) % 12;   // Return to first octave
        notes[3] = (cardId & 0xFF) % 24;          // Wide range for final note
        
        // Base frequencies optimized for cognitive recognition
        const uint16_t baseNotes[] = {
            PitchPerception::NOTE_C4, PitchPerception::NOTE_CS4, PitchPerception::NOTE_D4, 
            PitchPerception::NOTE_DS4, PitchPerception::NOTE_E4, PitchPerception::NOTE_F4,
            PitchPerception::NOTE_FS4, PitchPerception::NOTE_G4, PitchPerception::NOTE_GS4, 
            PitchPerception::NOTE_A4, PitchPerception::NOTE_AS4, PitchPerception::NOTE_B4,
            PitchPerception::NOTE_C5, PitchPerception::NOTE_CS5, PitchPerception::NOTE_D5, 
            PitchPerception::NOTE_DS5, PitchPerception::NOTE_E5, PitchPerception::NOTE_F5,
            PitchPerception::NOTE_FS5, PitchPerception::NOTE_G5, PitchPerception::NOTE_GS5, 
            PitchPerception::NOTE_A5, PitchPerception::NOTE_AS5, PitchPerception::NOTE_B5
        };
        
        // Set excited expression
        VisualCortex::RoverManager::setTemporaryExpression(
            PC::RoverTypes::Expression::EXCITED, 
            2000
        );
        
        // Play melody with dynamic timing
        for (int i = 0; i < MELODY_LENGTH; i++) 
        {
            uint8_t duration = 50 + (notes[i] % 100);  // Variable note length
            SoundFxManager::playTone(baseNotes[notes[i]], duration, i);
            delay(duration * 0.6);  // Overlap notes slightly for smoother transition
        }
    }

    void SoundFxManager::playTimerDropSound(CRGB color) 
    {
        // Map colors to musical scale degrees for cognitive association
        const int colorToNoteMap[] = {
            PitchPerception::NOTE_C4,  // Red - grounding
            PitchPerception::NOTE_D4,  // Orange - warmth
            PitchPerception::NOTE_E4,  // Yellow - brightness
            PitchPerception::NOTE_F4,  // Green - nature
            PitchPerception::NOTE_G4,  // Blue - depth
            PitchPerception::NOTE_A4,  // Indigo - mystery
            PitchPerception::NOTE_B4,  // Purple - complexity
            PitchPerception::NOTE_C5   // White - completion
        };
        
        int baseNote = PitchPerception::NOTE_C4;  // Default
        
        // Color to note mapping with improved psychological associations
        if (color == CRGB::Red) baseNote = colorToNoteMap[0];
        else if (color == CRGB::Orange) baseNote = colorToNoteMap[1];
        else if (color == CRGB::Yellow) baseNote = colorToNoteMap[2];
        else if (color == CRGB::Green) baseNote = colorToNoteMap[3];
        else if (color == CRGB::Blue) baseNote = colorToNoteMap[4];
        else if (color == CRGB::Indigo) baseNote = colorToNoteMap[5];
        else if (color == CRGB::Purple) baseNote = colorToNoteMap[6];
        else if (color == CRGB::White) baseNote = colorToNoteMap[7];
        
        // Initial clear note for attention
        SoundFxManager::playTone(baseNote, 50, 0);
        delay(10);
        
        // Enhanced water drop effect with harmonic series
        const int STEPS = 4;
        const int DURATION = 20;
        const int DROP_RANGE = 100;
        
        for (int i = 0; i < STEPS; i++) 
        {
            int pitch = baseNote - (DROP_RANGE >> i);  // Exponential pitch drop
            SoundFxManager::playTone(pitch, DURATION, 0);
            delay(DURATION - (i * 2));  // Accelerating tempo
        }
    }

    void SoundFxManager::playErrorCode(uint32_t errorCode, bool isFatal) {
        // Base frequencies for fatal vs warning
        uint16_t baseFreq = isFatal ? 440 : 880; // A4 for fatal, A5 for warning
        
        // Play binary representation of error code
        for (int i = 7; i >= 0; i--) {
            if (errorCode & (1 << i)) {
                playTone(baseFreq, 100);
            } else {
                playTone(baseFreq/2, 100);
            }
            delay(50);
        }
        
        // Final tone indicates fatal/warning
        if (isFatal) {
            playTone(220, 500); // Low A3 for fatal
        } else {
            playTone(1760, 200); // High A6 for warning
        }
    }

    void SoundFxManager::playToneFx(PC::AudioTypes::Tone type) 
    {
        switch (type) 
        {
            case PC::AudioTypes::Tone::SUCCESS:
                playSuccessSound();
                break;

            case PC::AudioTypes::Tone::ERROR:
                playErrorSound(ErrorSoundType::PLAYBACK);
                break;

            case PC::AudioTypes::Tone::WARNING:
                playErrorSound(ErrorSoundType::STORAGE);
                break;

            case PC::AudioTypes::Tone::NOTIFICATION:
                playTone(PitchPerception::NOTE_C5, 100);
                break;

            case PC::AudioTypes::Tone::TIMER_DROP:
                playTimerDropSound(CRGB::Blue);
                break;

            case PC::AudioTypes::Tone::LEVEL_UP:
                playSuccessSound();  // Could be customized for level up
                break;

            case PC::AudioTypes::Tone::GAME_OVER:
                playErrorSound(ErrorSoundType::PLAYBACK);
                break;

            case PC::AudioTypes::Tone::MENU_SELECT:
                playMenuSelectSound();
                break;

            case PC::AudioTypes::Tone::MENU_CHANGE:
                playRotaryTurnSound(true);
                break;

            case PC::AudioTypes::Tone::NONE:
            default:
                break;
        }
    }
}