#include "DataManager.h"
#include <LittleFS.h>

SemaphoreHandle_t DataManager::mutexNVS = xSemaphoreCreateMutex();
SemaphoreHandle_t DataManager::mutexI2C = xSemaphoreCreateMutex();

DataManager::DataManager() {
    setDefaultConfig();
}

DataManager& DataManager::getInstance() {
    static DataManager instance;
    return instance;
}

bool DataManager::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return false;
    }
    return loadConfig();
}

void DataManager::setDefaultConfig() {
    for (int i = 0; i < MAX_RELAYS; i++) {
        _config.relays[i].id = i + 1;
        _config.relays[i].state = 0;
        _config.relays[i].mode = "lighting";
        _config.relays[i].custom_name = "";
    }
    _config.system.lcd_enabled = true;
    _config.system.admin_token = "secret123";
    _config.system.relay_polarity = "LOW";
}

bool DataManager::loadConfig() {
    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(1000)) != pdTRUE) return false;

    bool result = false;
    if (LittleFS.exists(CONFIG_FILE_PATH)) {
        File file = LittleFS.open(CONFIG_FILE_PATH, "r");
        if (file) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, file);
            if (!error) {
                JsonArray relays = doc["relays"];
                int i = 0;
                for (JsonObject relay : relays) {
                    if (i < MAX_RELAYS) {
                        _config.relays[i].id = relay["id"] | (i + 1);
                        _config.relays[i].state = relay["state"] | 0;
                        _config.relays[i].mode = relay["mode"].as<const char*>() ? relay["mode"].as<String>() : "lighting";
                        _config.relays[i].custom_name = relay["custom_name"].as<const char*>() ? relay["custom_name"].as<String>() : "";
                        i++;
                    }
                }
                JsonObject system = doc["system"];
                _config.system.lcd_enabled = system["lcd_enabled"] | true;
                _config.system.admin_token = system["admin_token"].as<const char*>() ? system["admin_token"].as<String>() : "secret123";
                _config.system.relay_polarity = system["relay_polarity"].as<const char*>() ? system["relay_polarity"].as<String>() : "LOW";
                result = true;
            }
            file.close();
        }
    } else {
        xSemaphoreGive(mutexNVS);
        saveConfig();
        return true;
    }

    xSemaphoreGive(mutexNVS);
    return result;
}

bool DataManager::saveConfig() {
    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(1000)) != pdTRUE) return false;

    JsonDocument doc;
    JsonArray relays = doc["relays"].to<JsonArray>();
    for (int i = 0; i < MAX_RELAYS; i++) {
        JsonObject r = relays.add<JsonObject>();
        r["id"] = _config.relays[i].id;
        r["state"] = _config.relays[i].state;
        r["mode"] = _config.relays[i].mode;
        r["custom_name"] = _config.relays[i].custom_name;
    }

    JsonObject system = doc["system"].to<JsonObject>();
    system["lcd_enabled"] = _config.system.lcd_enabled;
    system["admin_token"] = _config.system.admin_token;
    system["relay_polarity"] = _config.system.relay_polarity;

    File file = LittleFS.open(CONFIG_FILE_PATH, "w");
    bool result = false;
    if (file) {
        if (serializeJson(doc, file) != 0) {
            result = true;
        }
        file.close();
    }

    xSemaphoreGive(mutexNVS);
    return result;
}

AppConfig DataManager::getConfig() {
    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(100)) == pdTRUE) {
        AppConfig cfg = _config;
        xSemaphoreGive(mutexNVS);
        return cfg;
    }
    return _config;
}

void DataManager::setRelayState(int id, int state) {
    if (id < 1 || id > MAX_RELAYS) return;
    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _config.relays[id - 1].state = state;
        xSemaphoreGive(mutexNVS);
        saveConfig();
    }
}

bool DataManager::setRelayConfig(int id, String mode, String custom_name) {
    if (id < 1 || id > MAX_RELAYS) return false;
    if (mode == "custom" && custom_name.length() > MAX_CUSTOM_NAME_LEN) return false;

    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _config.relays[id - 1].mode = mode;
        _config.relays[id - 1].custom_name = (mode == "custom") ? custom_name : "";
        xSemaphoreGive(mutexNVS);
        return saveConfig();
    }
    return false;
}

void DataManager::setLcdEnabled(bool enabled) {
    if (xSemaphoreTake(mutexNVS, pdMS_TO_TICKS(1000)) == pdTRUE) {
        _config.system.lcd_enabled = enabled;
        xSemaphoreGive(mutexNVS);
        saveConfig();
    }
}
