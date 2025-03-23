#include "HTTPServerManager.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

HTTPServerManager::HTTPServerManager(Logger* logger)
    : server(80), webSocket(81), _logger(logger) {}

void HTTPServerManager::begin() {
    if (!LittleFS.begin()) {
        if (_logger != nullptr) _logger->log("Failed to mount filesystem.\n");
        return;
    }

    // Initialize WebSocket
    webSocket.begin();
    webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        handleWebSocketEvent(num, type, payload, length);
    });

    // Serve static files from LittleFS
    server.onNotFound([this]() { handleFileRequest(); });

    // Start the server
    server.begin();
    if (_logger != nullptr) _logger->log("HTTP server started.\n");
}

void HTTPServerManager::loop() {
    server.handleClient();
    webSocket.loop();
}

/**
 * @brief Handles static files from the file system's directory: public_html/.
 *
 * @param server Reference to the ESP8266WebServer instance managing the request.
 *               Used to access request parameters and send responses.
 */
void HTTPServerManager::handleFileRequest() {
    String path = server.uri();
    if (path == "/") path = "/index.html";

    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";

    path = "public_html/" + path;
    if (LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    } else {
        server.send(404, "text/plain", "File Not Found");
    }
}

/**
 * @brief Allows easy integration of additional endpoints.
 *
 * @param uri A URI of an endpoint 
 * @param method HTTP method the request is made, for example its value can be HTTP_GET, HTTP_POST, etc.
 * @param handler Lambda function that should accept argument server, this is the way to get access to private attribute server of this class, from outside.
 * @param uploadHandler [optional] Lambda function as the above, used for file uploads 
 */
void HTTPServerManager::registerPage(
    const String& uri, 
    HTTPMethod method, 
    std::function<void(ESP8266WebServer&)> handler, 
    std::function<void(ESP8266WebServer&)> uploadHandler)
{
    if (uploadHandler) { // To support file uploads, you need to include a second handler for file uploads
        server.on(uri.c_str(), method, 
            [this, handler]() { 
                handler(server); 
            }, 
            [this, uploadHandler]() { 
                uploadHandler(server); 
            }
        );
    } else {
        server.on(uri.c_str(), method, [this, handler]() {
            handler(server);
        });
    }

}

/**
 * @brief 
 * @param message 
*/
void HTTPServerManager::broadcastWebSocketMessage(const String& message) {
    String mutableMessage = message; // Create a mutable copy of the const String
    webSocket.broadcastTXT(mutableMessage);
}

void HTTPServerManager::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
    case WStype_DISCONNECTED:
        Serial.printf("WebSocket [%u] disconnected\n", num);
        break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WebSocket [%u] connected from %s\n", num, ip.toString().c_str());
        break;
    }
    case WStype_TEXT:
        Serial.printf("WebSocket [%u] received: %s\n", num, payload);
        // Echo the received message
        String message = "Message received: " + String((char*)payload);
        webSocket.sendTXT(num, message);
        //webSocket.sendTXT(num, "Message received: " + String((char*)payload));
        break;
    }
}