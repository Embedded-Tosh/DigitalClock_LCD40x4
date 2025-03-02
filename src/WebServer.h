#ifndef MYSERVER_H
#define MYSERVER_H
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>

namespace WebServer
{
    File openFile(const char *name, const char *mode);
    void begin();
    void update();
    void onIndexRequest(AsyncWebServerRequest *request);
    void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t *payload, size_t length);
    void broadcast(uint8_t *payload);
}
#endif