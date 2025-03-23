#ifndef TELNET_LOGGER_H
#define TELNET_LOGGER_H

#include "Logger.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class TelnetLogger : public Logger {
public:
    // Constructor
    TelnetLogger(uint16_t port = 23);

    // Initialize the Telnet logger
    void begin() override;

    // Handle Telnet connections and log forwarding
    void loop() override;

    void log(const char* message) override;

private:
    WiFiServer telnetServer;  // Telnet server instance
    WiFiClient telnetClient;  // Telnet client instance

};

#endif