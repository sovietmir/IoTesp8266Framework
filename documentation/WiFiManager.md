# WiFi Manager Class

## Overview
The `WiFiManager` class provides comprehensive Wi-Fi connectivity management for ESP8266-based projects. It handles:
- Station mode (connecting to existing Wi-Fi networks)
- Access Point mode (creating a configurable hotspot)
- Automatic reconnection logic
- NTP time synchronization
- Web-based configuration via integrated HTTP server

## Key Features
- **Dual-mode operation**: Seamlessly switches between Station and AP modes
- **Self-healing**: Automatic reconnection attempts with configurable behavior
- **Time synchronization**: Built-in NTP client support with timezone configuration
- **Web configuration**: Built-in HTTP endpoints for network scanning and configuration
- **Event reporting**: Hook system for monitoring connection progress and errors
- **Logger integration**: Compatible with the Logger framework for status reporting

## Class Structure

### Enum: OperationMode
```cpp
enum OperationMode {
    INIT,      // Initial state before connecting
    NORMAL,    // Connected to Wi-Fi and operating normally
    SETTINGS   // Configuration mode (AP mode)
};
```

### Constructor
```cpp
WiFiManager(HTTPServerManager& serverManager, Logger* logger = nullptr)
```
- **Parameters**:
  - `serverManager`: Reference to HTTPServerManager for endpoint registration
  - `logger`: Optional Logger instance for status reporting

## Configuration Methods

| Method | Description |
|--------|-------------|
| `setSSID(const char*)` | Sets Wi-Fi network SSID |
| `setPassword(const char*)` | Sets Wi-Fi password |
| `setHostname(const char*)` | Sets device hostname |
| `setAPSSID(const char*)` | Sets AP mode SSID |
| `setAPPassword(const char*)` | Sets AP mode password |
| `setTimeServer(const char*)` | Sets NTP server address |
| `setTimeZone(const char*)` | Sets timezone string |

## Core Methods

### `void begin()`
Initializes the WiFiManager by:
1. Configuring NTP time synchronization
2. Attempting first connection to configured AP
3. Registering HTTP endpoints
4. Setting operation mode (NORMAL or SETTINGS)

### `void loop()`
Maintains network connection by:
- Handling mode transitions
- Managing AP mode when in SETTINGS mode
- Attempting reconnection when needed
- Checking connection status every 60 seconds

### Connection Methods
| Method | Description |
|--------|-------------|
| `firstConnectToAP()` | Initial connection attempt |
| `reConnecToAP()` | Reconnection attempt when disconnected |
| `connectToAP()` | Core connection logic |
| `createAP()` | Creates configuration access point |

### Utility Methods
| Method | Description |
|--------|-------------|
| `reboot()` | Restarts the ESP8266 |
| `addReportStepHook()` | Adds progress/error callback |
| `reportStep()` | Triggers all registered callbacks |
| `registerEndpoints()` | Registers web API endpoints |
| `handleScanAPs()` | Handles nearby AP scanning |

## HTTP Endpoints
- **GET /api/nearby-ap**: Returns JSON array of nearby access points

## Usage Example

### Basic Setup
```cpp
#include <Logger/ConsoleLogger.h>
#include <HTTPServerManager/HTTPServerManager.h>

#include <WiFiManager/WiFiManager.h>

ConsoleLogger logger;
HTTPServerManager serverManager;
WiFiManager wifiManager(serverManager, &logger);

void setup() {
  logger.begin();
  
  // Configure Wi-Fi
  wifiManager.setSSID("MyNetwork");
  wifiManager.setPassword("securepassword");
  wifiManager.setHostname("MyESP8266");
  
  // Configure AP fallback
  wifiManager.setAPSSID("ConfigAP");
  wifiManager.setAPPassword("configme");
  
  // Configure NTP
  wifiManager.setTimeServer("pool.ntp.org");
  wifiManager.setTimeZone("EET-2EEST,M3.5.0/3,M10.5.0/4"); // Athens timezone
  
  wifiManager.begin();
  serverManager.begin();
}

void loop() {
  wifiManager.loop();
  serverManager.loop();
}
```

### Adding Status Hooks
```cpp
void connectionHook(int step) {
  switch(step) {
    case 1: logger.log("Starting connection..."); break;
    case 4: logger.log("Connected successfully!"); break;
    case -1: logger.log("Error: Missing credentials"); break;
    case -2: logger.log("Error: Connection timeout"); break;
    case -4: logger.log("Error: Connection lost"); break;
  }
}

void setup() {
  // ... previous setup code
  wifiManager.addReportStepHook(connectionHook);
}
```

## Operation Modes

### NORMAL Mode
- Connected to configured Wi-Fi network
- NTP time synchronization active
- Web server available for configuration

### SETTINGS Mode
- Activated when:
  - Initial connection fails
  - Established connection is lost
- Creates configuration AP
- Web interface available for reconfiguration
- Periodically attempts to reconnect (every 60 seconds)

## Best Practices
1. Always configure both Station and AP credentials
2. Set a meaningful hostname for network identification
3. Use the hook system for detailed connection monitoring
4. For production, secure AP mode with a password
5. Choose geographically close NTP servers for better time accuracy

## Dependencies
- ESP8266WiFi (core library)
- HTTPServerManager (for web configuration)
- Logger (for status reporting) [optional]