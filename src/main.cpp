#include <Arduino.h>
#include <WiFi.h>
#include <WiFiProvisioner.h>
#include "Config.h"
#include "DataManager.h"
#include "Hardware.h"
#include "ApiServer.h"
#include "LcdInterface.h"

// Instantiate the class correctly
WiFiProvisioner wifiProvisioner;

void setup() {
    Serial.begin(115200);

    if (!DataManager::getInstance().begin()) {
        Serial.println("System halt: DataManager failed");
        while(1);
    }

    Hardware::getInstance().begin();

    // Use correct method names if known, or generic start
    // wifiProvisioner.start();

    xTaskCreatePinnedToCore(Task_Input, "InputTask", STACK_SIZE_SMALL, NULL, PRIO_INPUT, NULL, 1);
    xTaskCreatePinnedToCore(Task_LCD,   "LCDTask",   STACK_SIZE_MEDIUM, NULL, PRIO_LCD,   NULL, 1);
    xTaskCreatePinnedToCore(Task_Relay, "RelayTask", STACK_SIZE_MEDIUM, NULL, PRIO_RELAY, NULL, 1);
    xTaskCreatePinnedToCore(Task_API,   "APITask",   STACK_SIZE_LARGE,  NULL, PRIO_API,   NULL, 0);

    Serial.println("MAC-V2 Controller Started Successfully");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
