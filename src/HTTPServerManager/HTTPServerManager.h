#ifndef HTTP_SERVER_MANAGER_H
#define HTTP_SERVER_MANAGER_H

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "Logger/Logger.h"

class HTTPServerManager {
public:
    HTTPServerManager(Logger* logger = nullptr);

    void begin();    
    void loop(); // Handle HTTP server requests and WebSocket events

    // Register a custom route
    void registerPage(const String& uri, HTTPMethod method, std::function<void(ESP8266WebServer&)> handler, std::function<void(ESP8266WebServer&)> uploadHandler = nullptr);

    // Broadcast message to all WebSocket clients
    void broadcastWebSocketMessage(const String& message);

private:
    ESP8266WebServer server;
    WebSocketsServer webSocket;
    Logger* _logger;

    // Internal method to handle WebSocket events
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

    void handleFileRequest();
};

#endif