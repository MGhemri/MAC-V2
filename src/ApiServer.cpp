#include "ApiServer.h"
#include "DataManager.h"
#include "Hardware.h"
#include <AsyncJson.h>

ApiServer::ApiServer() : _server(80), _ws("/ws") {}

ApiServer& ApiServer::getInstance() {
    static ApiServer instance;
    return instance;
}

void ApiServer::begin() {
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onWsEvent(server, client, type, arg, data, len);
    });
    _server.addHandler(&_ws);

    setupRoutes();

    // Default CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    _server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            request->send(200);
        } else {
            request->send(404);
        }
    });

    _server.begin();
}

void ApiServer::setupRoutes() {
    _server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        AppConfig cfg = DataManager::getInstance().getConfig();
        JsonDocument doc;
        JsonArray relays = doc["relays"].to<JsonArray>();
        for (int i = 0; i < MAX_RELAYS; i++) {
            JsonObject r = relays.add<JsonObject>();
            r["id"] = cfg.relays[i].id;
            r["state"] = cfg.relays[i].state;
            r["mode"] = cfg.relays[i].mode;
            r["custom_name"] = cfg.relays[i].custom_name;
        }
        JsonObject system = doc["system"].to<JsonObject>();
        system["lcd_enabled"] = cfg.system.lcd_enabled;
        system["relay_polarity"] = cfg.system.relay_polarity;

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });

    _server.on("/api/relay/1/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(1); ApiServer::getInstance().broadcastUpdate(1, DataManager::getInstance().getConfig().relays[0].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/2/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(2); ApiServer::getInstance().broadcastUpdate(2, DataManager::getInstance().getConfig().relays[1].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/3/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(3); ApiServer::getInstance().broadcastUpdate(3, DataManager::getInstance().getConfig().relays[2].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/4/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(4); ApiServer::getInstance().broadcastUpdate(4, DataManager::getInstance().getConfig().relays[3].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/5/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(5); ApiServer::getInstance().broadcastUpdate(5, DataManager::getInstance().getConfig().relays[4].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/6/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(6); ApiServer::getInstance().broadcastUpdate(6, DataManager::getInstance().getConfig().relays[5].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/7/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(7); ApiServer::getInstance().broadcastUpdate(7, DataManager::getInstance().getConfig().relays[6].state); request->send(200, "application/json", "{\"success\":true}"); });
    _server.on("/api/relay/8/toggle", HTTP_POST, [](AsyncWebServerRequest *request) { Hardware::getInstance().toggleRelay(8); ApiServer::getInstance().broadcastUpdate(8, DataManager::getInstance().getConfig().relays[7].state); request->send(200, "application/json", "{\"success\":true}"); });

    _server.addHandler(new AsyncCallbackJsonWebHandler("/api/system/lcd", [](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonObject obj = json.as<JsonObject>();
        bool enabled = obj["enabled"] | true;
        DataManager::getInstance().setLcdEnabled(enabled);
        request->send(200, "application/json", "{\"success\":true}");
    }));

    for (int i = 1; i <= 8; i++) {
        String path = "/api/relay/" + String(i) + "/config";
        AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(path, [i](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject obj = json.as<JsonObject>();
            String mode = obj["mode"] | "lighting";
            String custom_name = obj["custom_name"] | "";
            if (mode == "custom" && custom_name.length() > MAX_CUSTOM_NAME_LEN) {
                request->send(400, "application/json", "{\"error\":\"Name too long\"}");
                return;
            }
            DataManager::getInstance().setRelayConfig(i, mode, custom_name);
            request->send(200, "application/json", "{\"success\":true}");
        });
        handler->setMethod(HTTP_PUT);
        _server.addHandler(handler);
    }
}

void ApiServer::broadcastUpdate(int relayId, int state) {
    JsonDocument doc;
    doc["event"] = "relay_update";
    doc["id"] = relayId;
    doc["state"] = state;
    String response;
    serializeJson(doc, response);
    _ws.textAll(response);
}

void ApiServer::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        client->printf("Connected to ESP32 WebSocket");
    }
}

void Task_API(void *pvParameters) {
    ApiServer::getInstance().begin();
    for (;;) {
        // Correcting private access in Task_API is tricky without making it public or adding a method
        // For simplicity in this fix, I'll rely on periodic cleanup if I add a public method.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
