#ifndef API_SERVER_H
#define API_SERVER_H

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

class ApiServer {
public:
    static ApiServer& getInstance();

    void begin();
    void broadcastUpdate(int relayId, int state);

private:
    ApiServer();
    AsyncWebServer _server;
    AsyncWebSocket _ws;

    void setupRoutes();
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

void Task_API(void *pvParameters);

#endif // API_SERVER_H
