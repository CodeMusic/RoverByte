#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

// LED Pins
#define LED_DATA_PIN 48  // Adjust this to your actual pin
#define LED_CLOCK_PIN 47 // Adjust this to your actual pin

// buttons
#define BOARD_USER_KEY 6
#define BOARD_PWR_EN   15

// WS2812
#define WS2812_NUM_LEDS 8
#define WS2812_DATA_PIN 14
#define RGB_ORDER GRB      // Define the RGB order


// IR
#define BOARD_IR_EN 2
#define BOARD_IR_RX 1

// MIC
#define BOARD_MIC_DATA 42
#define BOARD_MIC_CLK  39

// VOICE
// #define BOARD_VOICE_MODE  4
#define BOARD_VOICE_BCLK  46
#define BOARD_VOICE_LRCLK 40
#define BOARD_VOICE_DIN   7

// --------- DISPLAY ---------
// About LCD definition in the file: lib/TFT_eSPI/User_Setups/Setup214_LilyGo_T_Embed_PN532.h
// #define ST7789_DRIVER     // Configure all registers
// #define TFT_WIDTH  170
// #define TFT_HEIGHT 320

// #define TFT_BL     21   // LED back-light
// #define TFT_MISO   10   
// #define TFT_MOSI   9
// #define TFT_SCLK   11
// #define TFT_CS     41 
// #define TFT_DC     16
// #define TFT_RST    40 // Connect reset to ensure display initialises


// --------- ENCODER ---------
#define ENCODER_INA 4
#define ENCODER_INB 5
#define ENCODER_KEY 0

// --------- IIC ---------
#define BOARD_I2C_SDA  8
#define BOARD_I2C_SCL  18

// IIC addr
#define BOARD_I2C_ADDR_1 0x24  // PN532
#define BOARD_I2C_ADDR_2 0x55  // PMU
#define BOARD_I2C_ADDR_3 0x6b  // BQ25896

// NFC
#define BOARD_PN532_SCL     BOARD_I2C_SCL
#define BOARD_PN532_SDA     BOARD_I2C_SDA
#define BOARD_PN532_RF_REST 45
#define BOARD_PN532_IRQ     17

// --------- SPI ---------
#define BOARD_SPI_SCK  11
#define BOARD_SPI_MOSI 9
#define BOARD_SPI_MISO 10

// TF card
#define BOARD_SD_CS   13
#define BOARD_SD_SCK  BOARD_SPI_SCK
#define BOARD_SD_MOSI BOARD_SPI_MOSI
#define BOARD_SD_MISO BOARD_SPI_MISO

// LORA
#define BOARD_LORA_CS   12
#define BOARD_LORA_SCK  BOARD_SPI_SCK
#define BOARD_LORA_MOSI BOARD_SPI_MOSI
#define BOARD_LORA_MISO BOARD_SPI_MISO
#define BOARD_LORA_IO2  38
#define BOARD_LORA_IO0  3
#define BOARD_LORA_SW1  47
#define BOARD_LORA_SW0  48

// Core configuration
#define SCREEN_CENTER_X 85
#define CLOCK_PIN 45

// Add if not already present:
#define SD_CS    13  // SD Card CS pin
#define SD_MOSI  10  // SD Card MOSI pin
#define SD_MISO  9  // SD Card MISO pin
#define SD_SCK   11  // SD Card SCK pin    

#endif 