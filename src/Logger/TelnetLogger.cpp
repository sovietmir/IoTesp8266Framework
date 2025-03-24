#include "TelnetLogger.h"

// Constructor
TelnetLogger::TelnetLogger(uint16_t port) : telnetServer(port) {}

// Initialize the Telnet logger
void TelnetLogger::begin() {
    Serial.begin(115200); ///< This is needed since TelnetLogger::log method dublicates the output into Serial
    telnetServer.begin();
    telnetServer.setNoDelay(true);
    //Serial.println("Telnet server started");
}

// Handle Telnet connections and log forwarding
void TelnetLogger::loop() {
    // Accept new clients
    if (telnetServer.hasClient()) {
        if (telnetClient.connected()) {
            telnetClient.stop(); // Disconnect the existing client
            //Serial.println("Old Telnet client disconnected");
        }
        telnetClient = telnetServer.accept(); // Assign the new client
        //Serial.println("New Telnet client connected");
    }

    // Forward Serial data to the Telnet client
    if (telnetClient && telnetClient.connected()) {
        while (Serial.available()) {
            telnetClient.write(Serial.read());
        }
    }

    // Forward Telnet client data to Serial
    if (telnetClient && telnetClient.available()) {
        while (telnetClient.available()) {
            Serial.write(telnetClient.read());
        }
    }
}

// Log a message to the Telnet client
void TelnetLogger::log(const char* message) {
    Serial.print(message); // Log to the Serial Monitor
    if (telnetClient && telnetClient.connected()) {
        telnetClient.print(message); // Log to Telnet client
    }
}
