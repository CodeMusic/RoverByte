#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include <FastLED.h>

namespace MotorCortex
{
    /**
     * @brief Defines neural pathway connections for physical interfaces
     * 
     * Maps hardware connections to cognitive functions:
     * - Visual Processing (LED, Display)
     * - Auditory Processing (Audio I/O)
     * - Tactile Processing (Buttons, Encoder)
     * - Communication Pathways (I2C, SPI)
     */
    class PinDefinitions
    {
    public:
        // Visual Sensory System
        struct VisualPathways
        {
            // LED Pins
            static constexpr uint8_t LED_DATA_PIN = 48;
            static constexpr uint8_t LED_CLOCK_PIN = 47;

            // WS2812 Configuration
            static constexpr uint8_t WS2812_DATA_PIN = 14;
            static constexpr uint8_t WS2812_NUM_LEDS = 8;
            static constexpr EOrder WS2812_COLOR_ORDER = GRB;

            // LED function-specific indices
            #define LED_ERROR_START 0
            #define LED_ERROR_COUNT 8
            #define LED_LOADING_STEP 3

            // Display Configuration
            // About LCD definition in the file: lib/TFT_eSPI/User_Setups/Setup214_LilyGo_T_Embed_PN532.h
            // #define ST7789_DRIVER     // Configure all registers
            // #define TFT_WIDTH  170
            // #define TFT_HEIGHT 320
            // #define TFT_BL     21   // LED back-light
            // #define TFT_MISO   10   
            // #define TFT_MOSI   9
            // #define TFT_SCLK   11
            #define TFT_CS     41 
            // #define TFT_DC     16
            // #define TFT_RST    40 // Connect reset to ensure display initialises
            //#define SCREEN_CENTER_X 85
        };

        // Auditory Sensory System
        struct AuditoryPathways
        {
            // MIC
            #define BOARD_MIC_DATA 42
            #define BOARD_MIC_CLK  39

            // VOICE
            // #define BOARD_VOICE_MODE  4
            #define BOARD_VOICE_BCLK  46
            #define BOARD_VOICE_LRCLK 40
            #define BOARD_VOICE_DIN   7
        };

        // Tactile Sensory System
        struct TactilePathways
        {
            // Buttons
            #define BOARD_USER_KEY 6
            #define BOARD_PWR_EN   15

            // Encoder
            #define ENCODER_INA 4
            #define ENCODER_INB 5
            #define ENCODER_KEY 0
        };

        // Communication Pathways
        struct CommunicationPathways
        {
            // IR
            #define BOARD_IR_EN 2
            #define BOARD_IR_RX 1

            // I2C
            #define BOARD_I2C_SDA  8
            #define BOARD_I2C_SCL  18

            // I2C Addresses
            #define BOARD_I2C_ADDR_1 0x24  // PN532
            #define BOARD_I2C_ADDR_2 0x55  // PMU
            #define BOARD_I2C_ADDR_3 0x6b  // BQ25896

            // SPI
            #define BOARD_SPI_SCK  11
            #define BOARD_SPI_MOSI 9
            #define BOARD_SPI_MISO 10

            // NFC
            #define BOARD_PN532_SCL     BOARD_I2C_SCL
            #define BOARD_PN532_SDA     BOARD_I2C_SDA
            #define BOARD_PN532_RF_REST 45
            #define BOARD_PN532_IRQ     17
            #define SDA_PIN 4
            #define SCL_PIN 5
        };

        // Storage System
        struct StoragePathways
        {
            // TF card
            #define BOARD_SD_CS   13
            #define BOARD_SD_SCK  BOARD_SPI_SCK
            #define BOARD_SD_MOSI BOARD_SPI_MOSI
            #define BOARD_SD_MISO BOARD_SPI_MISO

            // Additional SD Card pins
            #define SD_CS    13  // SD Card CS pin
            #define SD_MOSI  10  // SD Card MOSI pin
            #define SD_MISO  9   // SD Card MISO pin
            #define SD_SCK   11  // SD Card SCK pin    
        };

        // LORA Communication
        struct LoRaPathways
        {
            #define BOARD_LORA_CS   12
            #define BOARD_LORA_SCK  BOARD_SPI_SCK
            #define BOARD_LORA_MOSI BOARD_SPI_MOSI
            #define BOARD_LORA_MISO BOARD_SPI_MISO
            #define BOARD_LORA_IO2  38
            #define BOARD_LORA_IO0  3
            #define BOARD_LORA_SW1  47
            #define BOARD_LORA_SW0  48
        };

        // Core configuration
        #define CLOCK_PIN 45
    };
}

#endif // PIN_DEFINITIONS_H 