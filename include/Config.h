#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PINOUT ---
// Encoder KY-040
#define PIN_ENCODER_CLK 16
#define PIN_ENCODER_DT  17
#define PIN_ENCODER_SW  18

// LCD I2C
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22
#define LCD_ADDR        0x27
#define LCD_COLS        20
#define LCD_ROWS        4

// Relays
const uint8_t RELAY_PINS[8] = {2, 4, 5, 12, 13, 14, 15, 27};

// --- SYSTEM CONSTANTS ---
#define MAX_RELAYS          8
#define MAX_CUSTOM_NAME_LEN 8
#define DEBOUNCE_TIME_MS    50
#define LCD_TIMEOUT_MS      30000
#define CONFIG_FILE_PATH    "/config.json"

// --- FreeRTOS SETTINGS ---
#define STACK_SIZE_SMALL    2048
#define STACK_SIZE_MEDIUM   4096
#define STACK_SIZE_LARGE    8192

#define PRIO_INPUT          4
#define PRIO_LCD            3
#define PRIO_RELAY          3
#define PRIO_API            2

// Event Queue Sizes
#define QUEUE_INPUT_SIZE    10

enum class InputEvent {
    UP,
    DOWN,
    CLICK,
    LONG_CLICK
};

#endif // CONFIG_H
