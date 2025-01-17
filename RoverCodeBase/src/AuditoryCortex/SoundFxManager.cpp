#include "../AuditoryCortex/PitchPerception.h"
#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>
#include "../PrefrontalCortex/SDManager.h"
#include "../VisualCortex/RoverViewManager.h"
#include "../VisualCortex/RoverManager.h"
#include "../VisualCortex/LEDManager.h"


// Initialize static members
int SoundFxManager::currentNote = 0;
unsigned long SoundFxManager::lastNoteTime = 0;
bool SoundFxManager::jinglePlaying = false;
Audio SoundFxManager::audio;
bool SoundFxManager::isPlayingSound = false;
const char* SoundFxManager::RECORD_FILENAME = "/sdcard/temp_record.wav";
bool SoundFxManager::isRecording = false;
File SoundFxManager::recordFile;

// Add after the other jingle definitions
const SoundFxManager::Note SoundFxManager::ROVERBYTE_JINGLE[] = {
    {PitchPerception::NOTE_G4, 100, 30},    // G - Base note
    {PitchPerception::NOTE_C5, 100, 30},    // C - Up to C
    {PitchPerception::NOTE_E5, 100, 30},    // E - Complete the C major triad
    {PitchPerception::NOTE_G5, 150, 50},    // G - Octave up, held longer
    {PitchPerception::NOTE_E5, 100, 30},    // E - Back down
    {PitchPerception::NOTE_C5, 100, 30},    // C - Continue down
    {PitchPerception::NOTE_D5, 150, 50},    // D - Surprise note
    {PitchPerception::NOTE_G5, 200, 0}      // G - Final high note, held longest
};

const int SoundFxManager::JINGLE_LENGTH = sizeof(SoundFxManager::ROVERBYTE_JINGLE) / sizeof(SoundFxManager::Note);

void SoundFxManager::playTone(int frequency, int duration, int position) {
    // ESP32 LEDC limitations: freq_hz * duty_resolution < 80MHz
    // Using 8-bit resolution, so max frequency should be around 312.5KHz
    
    // Clamp frequency to safe range
    if (frequency < 100) frequency = 100;  // Minimum frequency
    if (frequency > 10000) frequency = 10000;  // Maximum frequency
    
    ledcSetup(TONE_PWM_CHANNEL, frequency, 8);  // 8-bit resolution
    ledcAttachPin(BOARD_VOICE_DIN, TONE_PWM_CHANNEL);
    ledcWrite(TONE_PWM_CHANNEL, 127);  // 50% duty cycle
    delay(duration);
    ledcWrite(TONE_PWM_CHANNEL, 0);
    ledcDetachPin(BOARD_VOICE_DIN);

    //LEDManager::getNoteColor(PitchPerception::getNoteInfo(frequency));
    LEDManager::displayNote(frequency, position);
}

void SoundFxManager::startJingle() {
    currentNote = 0;
    jinglePlaying = true;
    lastNoteTime = 0;
}

void SoundFxManager::updateJingle() {
    if (!jinglePlaying || currentNote >= JINGLE_LENGTH) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastNoteTime >= ROVERBYTE_JINGLE[currentNote].duration + ROVERBYTE_JINGLE[currentNote].delay) {
        if (currentNote < JINGLE_LENGTH) {
            SoundFxManager::playTone(SoundFxManager::ROVERBYTE_JINGLE[currentNote].pitch, SoundFxManager::ROVERBYTE_JINGLE[currentNote].duration);
            lastNoteTime = currentTime;
            currentNote++;
        } else {
            SoundFxManager::stopJingle();
        }
    }
}

bool SoundFxManager::isJinglePlaying() {
    return jinglePlaying;
}

void SoundFxManager::playSuccessSound() {
    playTone(PitchPerception::NOTE_C5, 100);
    delay(50);
    playTone(PitchPerception::NOTE_E5, 100);
    delay(50);
    playTone(PitchPerception::NOTE_G5, 200);
}

void SoundFxManager::playRotaryPressSound(int mode) {  // 0=Full, 1=Week, 2=Timer
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayOfWeek = timeInfo->tm_wday;  // 0-6 (Sunday-Saturday)
    uint16_t baseNote = PitchPerception::getDayBaseNote(mode == 1); // Full mode - base octave // Week mode - octave up

    
    // Adjust octave based on mode
    switch(mode) {
        case 0: 
            SoundFxManager::playTone(baseNote, 100);
            break;
        case 1:  
            SoundFxManager::playTone(baseNote, 100);  
            break;
        case 2:  // Timer mode - octave up + fifth
            SoundFxManager::playTone(baseNote * 2, 100);  // Multiply by 3 for octave 
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

void SoundFxManager::playErrorSound(int type) {
    switch(type) {
        case 1: // Recording error
            SoundFxManager::playTone(PitchPerception::NOTE_B5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_D5, 400);
            break;
            
        case 2: // SD card error
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_G5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_G4, 400);
            break;
            
        case 3: // Playback error
            SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_D5, 200);
            delay(100);
            SoundFxManager::playTone(PitchPerception::NOTE_D4, 400);
            break;
    }
}

void SoundFxManager::playStartupSound() {
    if (!SPIFFS.exists("/initialized.txt")) {
        // Only play on first boot
        for (int i = 0; i < SoundFxManager::JINGLE_LENGTH; i++) {
            SoundFxManager::playTone(SoundFxManager::ROVERBYTE_JINGLE[i].pitch, SoundFxManager::ROVERBYTE_JINGLE[i].duration);
            if (SoundFxManager::ROVERBYTE_JINGLE[i].delay > 0) {
                delay(SoundFxManager::ROVERBYTE_JINGLE[i].delay);
            }
        }
        
        // Create initialization file
        File f = SPIFFS.open("/initialized.txt", "w");
        if (f) {
            f.println("initialized");
            f.close();
        }
    }
}

void SoundFxManager::playJingle() {
    if (!jinglePlaying) {
        jinglePlaying = true;
        currentNote = 0;
        lastNoteTime = millis();
    }
}


// Add audio callback
void SoundFxManager::audio_eof_mp3(const char *info) {
    Serial.printf("Audio playback finished: %s\n", info);
    // Delete temporary recording after playback
    if (!SD.remove(RECORD_FILENAME)) {
        Serial.println("Failed to delete temporary recording file");
        playErrorSound(2);
    }
    isPlayingSound = false;
}


void SoundFxManager::startRecording() {
    if (!SDManager::isInitialized()) return;
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
        SoundFxManager::playErrorSound(1);
        RoverManager::setEarsDown();
        return;
    }
    
    if (i2s_set_pin((i2s_port_t)EXAMPLE_I2S_CH, &pin_config) != ESP_OK) {
        Serial.println("ERROR: Failed to set I2S pins");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(1);
        RoverManager::setEarsDown();
        return;
    }

    // Create new WAV file
    recordFile = SD.open(RECORD_FILENAME, FILE_WRITE);
    if (!recordFile) {
        Serial.println("ERROR: Failed to open file for recording");
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        RoverManager::setEarsDown();
        return;
    }

    // Reserve space for the header
    for (int i = 0; i < WAVE_HEADER_SIZE; i++) {
        recordFile.write(0);
    }

    isRecording = true;
    Serial.println("Recording started!");
}

void SoundFxManager::stopRecording() {
    if (!isRecording) return;

    Serial.println("=== Stopping Recording ===");
    isRecording = false;
    
    // Get final size
    uint32_t file_size = recordFile.size() - WAVE_HEADER_SIZE;
    
    // Generate and write header
    char wav_header[WAVE_HEADER_SIZE];
    generate_wav_header(wav_header, file_size, EXAMPLE_SAMPLE_RATE);
    
    if (!recordFile.seek(0)) {
        Serial.println("ERROR: Failed to seek in file");
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        RoverManager::setEarsDown();
        return;
    }
    
    if (recordFile.write((uint8_t *)wav_header, WAVE_HEADER_SIZE) != WAVE_HEADER_SIZE) {
        Serial.println("ERROR: Failed to write WAV header");
        recordFile.close();
        i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);
        SoundFxManager::playErrorSound(2);
        RoverManager::setEarsDown();
        return;
    }
    
    recordFile.close();
    i2s_driver_uninstall((i2s_port_t)EXAMPLE_I2S_CH);

    // Play the recording
    Serial.println("Attempting to play recording...");
    audio.setVolume(21);
    
    if (SD.exists(RECORD_FILENAME)) {
        Serial.println("Found recording file, playing...");
        if (!audio.connecttoFS(SD, RECORD_FILENAME)) {
            Serial.println("ERROR: Failed to start playback");
            SoundFxManager::playErrorSound(3);
            RoverManager::setEarsDown();
            return;
        }
    } else {
        Serial.println("ERROR: Recording file not found!");
        SoundFxManager::playErrorSound(3);
        RoverManager::setEarsDown();
        return;
    }
    
    SDManager::closeFile(recordFile);
    RoverManager::setEarsDown();  // Put ears down after successful recording/playback start
    
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
    // See this for reference: http://soundfile.sapp.org/doc/WaveFormat/
    uint32_t file_size = wav_size + WAVE_HEADER_SIZE - 8;
    uint32_t byte_rate = BYTE_RATE;

    const char set_wav_header[] = {
        'R','I','F','F', // ChunkID
        (char)file_size, (char)(file_size >> 8), (char)(file_size >> 16), (char)(file_size >> 24), // ChunkSize
        'W','A','V','E', // Format
        'f','m','t',' ', // Subchunk1ID
        0x10, 0x00, 0x00, 0x00, // Subchunk1Size (16 for PCM)
        0x01, 0x00, // AudioFormat (1 for PCM)
        0x01, 0x00, // NumChannels (1 channel)
        (char)sample_rate, (char)(sample_rate >> 8), (char)(sample_rate >> 16), (char)(sample_rate >> 24), // SampleRate
        (char)byte_rate, (char)(byte_rate >> 8), (char)(byte_rate >> 16), (char)(byte_rate >> 24), // ByteRate
        0x02, 0x00, // BlockAlign
        0x10, 0x00, // BitsPerSample (16 bits)
        'd','a','t','a', // Subchunk2ID
        (char)wav_size, (char)(wav_size >> 8), (char)(wav_size >> 16), (char)(wav_size >> 24), // Subchunk2Size
    };

    memcpy(wav_header, set_wav_header, sizeof(set_wav_header));
}

void SoundFxManager::init() {
    // Initialize audio hardware
    audio.setPinout(BOARD_VOICE_BCLK, BOARD_VOICE_LRCLK, BOARD_VOICE_DIN);
    audio.setVolume(42);  // Set a reasonable volume level
    
    // Initialize SPIFFS for sound files
    if (!SPIFFS.begin(true)) {
        LOG_ERROR("Failed to initialize SPIFFS");
        return;
    }
    
    // Initialize I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 32,
        .use_apll = false
    };
    
    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK) {
        LOG_PROD("I2S driver installed successfully");
    } else {
        LOG_ERROR("I2S driver installation failed");
    }
    
    playStartupSound();
}

void SoundFxManager::stopJingle() {
    jinglePlaying = false;
    currentNote = 0;
    lastNoteTime = 0;
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
    
}

void SoundFxManager::playCardMelody(uint32_t cardId) {
    // Generate melody from all UID bytes
    uint8_t notes[7];  // Support for full 7-byte UID
    notes[0] = ((cardId >> 24) & 0xFF) % 24;
    notes[1] = ((cardId >> 16) & 0xFF) % 24;
    notes[2] = ((cardId >> 8) & 0xFF) % 24;
    notes[3] = (cardId & 0xFF) % 24;
    
    // Base note frequencies for C4 to B5 (two octaves)
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
    
    // Set excited expression before playing
    RoverManager::setTemporaryExpression(RoverManager::EXCITED, 2000);  // Show excited face for 2 seconds
    
    // Play the generated melody
    for (int i = 0; i < 4; i++) {
        uint8_t duration = 50 + (notes[i] % 100);
        SoundFxManager::playTone(baseNotes[notes[i]], duration, i);
        delay(duration / 2);
    }
}

void SoundFxManager::playTimerDropSound(CRGB color) {
    // Convert color to musical note (using same base notes)
    int baseNote;
    if (color == CRGB::Red) baseNote = PitchPerception::NOTE_C4;
    else if (color == CRGB::Orange) baseNote = PitchPerception::NOTE_D4;
    else if (color == CRGB::Yellow) baseNote = PitchPerception::NOTE_E4;
    else if (color == CRGB::Green) baseNote = PitchPerception::NOTE_F4;
    else if (color == CRGB::Blue) baseNote = PitchPerception::NOTE_G4;
    else if (color == CRGB::Indigo) baseNote = PitchPerception::NOTE_A4;
    else if (color == CRGB::Purple) baseNote = PitchPerception::NOTE_B4;
    else if (color == CRGB::White) baseNote = PitchPerception::NOTE_C5;
    else baseNote = PitchPerception::NOTE_C4;

    // Play initial clear note
    SoundFxManager::playTone(baseNote, 50, 0);
    delay(10);

    // Create gentler water drop effect
    const int steps = 4;
    const int duration = 20;
    const int dropRange = 100;
    
    for (int i = 0; i < steps; i++) {
        int freq = baseNote - (dropRange * i / steps);
        SoundFxManager::playTone(freq, duration);
        delay(5);
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

