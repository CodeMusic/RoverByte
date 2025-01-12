#include "SoundFxManager.h"
#include "Arduino.h"
#include <time.h>
#include <SPIFFS.h>

// Initialize static members
int SoundFxManager::currentNote = 0;
unsigned long SoundFxManager::lastNoteTime = 0;
bool SoundFxManager::jinglePlaying = false;
Audio SoundFxManager::audio;
bool SoundFxManager::isPlayingSound = false;
const char* SoundFxManager::RECORD_FILENAME = "/sdcard/temp_record.wav";


const SoundFxManager::Note SoundFxManager::ROVERBYTE_JINGLE[] = {
    // Opening flourish
    {NOTE_G4, 150, 50},   // G
    {NOTE_B4, 150, 50},   // B
    {NOTE_D5, 150, 50},   // D
    {NOTE_G5, 250, 100},  // High G
    
    // Playful melody
    {NOTE_E5, 200, 50},   // E
    {NOTE_C5, 200, 50},   // C
    {NOTE_D5, 200, 50},   // D
    {NOTE_B4, 200, 100},  // B
    
    // Bridge
    {NOTE_G4, 150, 50},   // G
    {NOTE_A4, 150, 50},   // A
    {NOTE_B4, 150, 50},   // B
    {NOTE_C5, 250, 100},  // C
    
    // Final part
    {NOTE_D5, 200, 50},   // D
    {NOTE_E5, 200, 50},   // E
    {NOTE_G5, 300, 100},  // High G
    {NOTE_G4, 400, 0}     // End on low G
};

const int SoundFxManager::JINGLE_LENGTH = sizeof(ROVERBYTE_JINGLE) / sizeof(Note);

const SoundFxManager::Note SoundFxManager::STARTUP_JINGLE[] = {
    {NOTE_G5, 50, 30},   // High G
    {NOTE_B5, 50, 30},   // Higher B
    {NOTE_D6, 50, 30},   // Even higher D
    {NOTE_G6, 100, 0}    // Highest G - final note
};

const int SoundFxManager::STARTUP_JINGLE_LENGTH = sizeof(STARTUP_JINGLE) / sizeof(Note);

const SoundFxManager::Note SoundFxManager::ROVER_JINGLE[] = {
    {NOTE_G4, 100, 0},    // G
    {NOTE_E5, 100, 0},    // E
    {NOTE_G5, 100, 0},    // G
    {NOTE_B5, 200, 50},   // B (held longer)
    {NOTE_A5, 100, 0},    // A
    {NOTE_G5, 200, 0}     // G (final note)
};
const int SoundFxManager::ROVER_JINGLE_LENGTH = 6;

void SoundFxManager::playTone(int frequency, int duration) {
    ledcSetup(TONE_PWM_CHANNEL, frequency, 8);
    ledcAttachPin(BOARD_VOICE_DIN, TONE_PWM_CHANNEL);
    ledcWrite(TONE_PWM_CHANNEL, 127);
    delay(duration);
    ledcWrite(TONE_PWM_CHANNEL, 0);
    ledcDetachPin(BOARD_VOICE_DIN);
}

void SoundFxManager::startJingle() {
    currentNote = 0;
    jinglePlaying = true;
    lastNoteTime = 0;
}

void SoundFxManager::updateJingle() {
    if (!jinglePlaying) return;
    
    unsigned long currentTime = millis();
    
    if (lastNoteTime == 0 || 
        (currentTime - lastNoteTime >= ROVERBYTE_JINGLE[currentNote].duration + 
                                     ROVERBYTE_JINGLE[currentNote].delay)) {
        
        playTone(ROVERBYTE_JINGLE[currentNote].pitch, 
                ROVERBYTE_JINGLE[currentNote].duration);
        
        lastNoteTime = currentTime;
        currentNote++;
        
        if (currentNote >= JINGLE_LENGTH) {
            jinglePlaying = false;
            currentNote = 0;
        }
    }
}

bool SoundFxManager::isJinglePlaying() {
    return jinglePlaying;
}

void SoundFxManager::playSuccessSound() {
    playTone(NOTE_C5, 100);
    delay(50);
    playTone(NOTE_E5, 100);
    delay(50);
    playTone(NOTE_G5, 200);
}

void SoundFxManager::playRotaryPressSound(int mode) {  // 0=Full, 1=Week, 2=Timer
    time_t now = time(nullptr);
    struct tm* timeInfo = localtime(&now);
    int dayOfWeek = timeInfo->tm_wday;  // 0-6 (Sunday-Saturday)
    
    // Base notes for each day (C through B)
    const int dayNotes[] = {
        NOTE_C4,  // Sunday
        NOTE_D4,  // Monday
        NOTE_E4,  // Tuesday
        NOTE_F4,  // Wednesday
        NOTE_G4,  // Thursday
        NOTE_A4,  // Friday
        NOTE_B4   // Saturday
    };
    
    // Get base note from current day
    int baseNote = dayNotes[dayOfWeek];
    
    // Adjust octave based on mode
    switch(mode) {
        case 0:  // Full mode - base octave
            playTone(baseNote, 100);
            break;
        case 1:  // Week mode - octave up
            playTone(baseNote * 2, 100);  // Multiply by 2 to go up an octave
            break;
        case 2:  // Timer mode - octave up + fifth
            playTone(baseNote * 3, 100);  // Multiply by 3 for octave + fifth
            break;
    }
}

void SoundFxManager::playRotaryTurnSound(bool clockwise) {
    if (clockwise) {
        playTone(getDayBaseNote4(), 50);
        playTone(getDayBaseNote5(), 50);
    } else {
        playTone(getDayBaseNote5(), 50);
        playTone(getDayBaseNote4(), 50);
    }
}

void SoundFxManager::playSideButtonSound(bool start) {
    if (start) {
        playTone(getDayBaseNote4(), 50);
        playTone(getDayBaseNote4(), 50);
    } else {
        playTone(getDayBaseNote5(), 100);
        int baseNote = getDayBaseNote5();
        playTone(getNoteMinus2(baseNote), 100);
    }
}

void SoundFxManager::playErrorSound(int type) {
    switch(type) {
        case 1: // Recording error
            playTone(NOTE_B5, 200);
            delay(100);
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_D5, 400);
            break;
            
        case 2: // SD card error
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_G5, 200);
            delay(100);
            playTone(NOTE_G4, 400);
            break;
            
        case 3: // Playback error
            playTone(NOTE_D5, 200);
            delay(100);
            playTone(NOTE_D5, 200);
            delay(100);
            playTone(NOTE_D4, 400);
            break;
    }
}

void SoundFxManager::playStartupSound() {
    if (!SPIFFS.exists("/initialized.txt")) {
        // Only play on first boot
        for (int i = 0; i < STARTUP_JINGLE_LENGTH; i++) {
            playTone(STARTUP_JINGLE[i].pitch, STARTUP_JINGLE[i].duration);
            if (STARTUP_JINGLE[i].delay > 0) {
                delay(STARTUP_JINGLE[i].delay);
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
    for (int i = 0; i < ROVER_JINGLE_LENGTH; i++) {
        playTone(ROVER_JINGLE[i].pitch, ROVER_JINGLE[i].duration);
        if (ROVER_JINGLE[i].delay > 0) {
            delay(ROVER_JINGLE[i].delay);
        }
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

void SoundFxManager::initializeAudio()
{
        // Initialize audio
    audio.setPinout(BOARD_VOICE_BCLK, BOARD_VOICE_LRCLK, BOARD_VOICE_DIN);
    audio.setVolume(21); // 0...21
    Serial.println("Audio initialized");
}

void SoundFxManager::init() {
    // Initialize SPIFFS for sound files
    if (!SPIFFS.begin(true)) {
        LOG_ERROR("Failed to initialize SPIFFS");
        return;
    }

    // Initialize audio hardware
    pinMode(BOARD_VOICE_DIN, OUTPUT);
    ledcSetup(TONE_PWM_CHANNEL, 5000, 8);  // 5KHz frequency, 8-bit resolution
    ledcAttachPin(BOARD_VOICE_DIN, TONE_PWM_CHANNEL);

    // Initialize microphone if needed
    init_microphone();

    // Play startup sound
    playStartupSound();

    LOG_DEBUG("SoundFxManager initialized");
}

