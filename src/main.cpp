#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "DataManager.h"
#include "Hardware.h"
#include "ApiServer.h"
#include "LcdInterface.h"

void setupWiFi() {
    AppConfig cfg = DataManager::getInstance().getConfig();

    if (cfg.system.wifi_ssid != "") {
        Serial.print("Connecting to WiFi: ");
        Serial.println(cfg.system.wifi_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(cfg.system.wifi_ssid.c_str(), cfg.system.wifi_pass.c_str());

        // Wait up to 10 seconds
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi Connected");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());

            LcdInterface::getInstance().showConnectionInfo();

            if (MDNS.begin("mac-controller")) {
                Serial.println("mDNS responder started: mac-controller.local");
            }
            return;
        }
        Serial.println("\nConnection failed. Falling back to AP mode.");
    }

    // Start AP mode if no credentials or connection fails
    Serial.println("Starting Access Point: MAC-V2-CONFIG");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MAC-V2-CONFIG", NULL); // Open AP for initial setup
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

    if (MDNS.begin("mac-controller")) {
        MDNS.addService("http", "tcp", 80);
    }
}

void setup() {
    Serial.begin(115200);

    if (!DataManager::getInstance().begin()) {
        Serial.println("System halt: DataManager failed");
        while(1);
    }

    Hardware::getInstance().begin();

    setupWiFi();

    xTaskCreatePinnedToCore(Task_Input, "InputTask", STACK_SIZE_SMALL, NULL, PRIO_INPUT, NULL, 1);
    xTaskCreatePinnedToCore(Task_LCD,   "LCDTask",   STACK_SIZE_MEDIUM, NULL, PRIO_LCD,   NULL, 1);
    xTaskCreatePinnedToCore(Task_Relay, "RelayTask", STACK_SIZE_MEDIUM, NULL, PRIO_RELAY, NULL, 1);
    xTaskCreatePinnedToCore(Task_API,   "APITask",   STACK_SIZE_LARGE,  NULL, PRIO_API,   NULL, 0);

    Serial.println("MAC-V2 Controller Started Successfully");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
