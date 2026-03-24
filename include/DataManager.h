#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Config.h"

struct RelayConfig {
    int id;
    int state;
    String mode;
    String custom_name;
};

struct SystemConfig {
    bool lcd_enabled;
    String admin_token;
    String relay_polarity;
};

struct AppConfig {
    RelayConfig relays[MAX_RELAYS];
    SystemConfig system;
};

class DataManager {
public:
    static DataManager& getInstance();

    bool begin();
    bool loadConfig();
    bool saveConfig();

    // Thread-safe accessors
    AppConfig getConfig();
    void setRelayState(int id, int state);
    bool setRelayConfig(int id, String mode, String custom_name);
    void setLcdEnabled(bool enabled);

    // Mutex for I2C and NVS/LittleFS
    static SemaphoreHandle_t mutexNVS;
    static SemaphoreHandle_t mutexI2C;

private:
    DataManager();
    AppConfig _config;
    void setDefaultConfig();
};

#endif // DATA_MANAGER_H
