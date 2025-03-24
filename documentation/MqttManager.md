# MQTT Manager Class

## Overview
The `MqttManager` class is a **wrapper for the `knolleary/PubSubClient` library**, designed to integrate MQTT communication in ESP8266 projects. It abstracts the  underlying PubSubClient, adds features like logging integration, topic management, and a callback system for reporting execution steps. 

---

## Key Features
1. **Wrapper for PubSubClient**: Built atop the [`knolleary/PubSubClient`](https://github.com/knolleary/pubsubclient) library, providing a higher-level interface for MQTT operations.
2. **Logger Integration**: Seamlessly works with the `Logger` framework (e.g., `ConsoleLogger`, `TelnetLogger`) for event logging.
3. **Automatic Reconnection**: Handles broker reconnection automatically in the `loop()` method.
4. **Topic Management**: Simplifies topic construction with a configurable base prefix (e.g., `EnergyMonitor/Device1/status`).
5. **Callback Hooks**: Supports user-defined callbacks for reporting errors or progress steps.

---

## Class: MqttManager

### Constructor
```cpp
MqttManager(Logger* logger);
```
- **Parameters**:
  - `logger`: Pointer to a `Logger` instance (e.g., `ConsoleLogger` or `TelnetLogger`).
- **Description**: Initializes the MQTT manager with a logger and sets up the underlying WiFi and MQTT clients. Uses `PubSubClient` (via `WiFiClient`) internally.

---

### Public Methods

#### `void begin()`
- **Description**: Description: Initializes the MQTT client with the configured server and port. Must be called after setting the server and port.

#### `void loop()`
- **Description**: Maintains the MQTT connection and processes incoming messages. Automatically reconnects to the broker if disconnected.


#### `void setTopicPrefix(const char* IoTclassName, const char* IoTName)`
- **Parameters**:
  - `IoTclassName`: The class name for the IoT device (e.g., "EnergyMonitor").
  - `IoTName`: The unique name of the IoT device (e.g., "Device1").
- **Description**: Sets the base topic prefix for MQTT messages in the format `IoTclassName/IoTName`.

#### `void setServer(const char* value)`
- **Parameters**:
  - `value`: The MQTT broker's IP address or hostname.
- **Description**: Sets the MQTT broker server address.

#### `void setPort(int value)`
- **Parameters**:
  - `value`: The MQTT broker's port number.
- **Description**: Sets the MQTT broker port.

#### `void setClientId(const char* value)`
- **Parameters**:
  - `value`: The unique client identifier for the MQTT connection.
- **Description**: Sets the MQTT client ID.

#### `void setUsername(const char* value)`
- **Parameters**:
  - `value`: The username for MQTT authentication.
- **Description**: Sets the MQTT username.

#### `void setPassword(const char* value)`
- **Parameters**:
  - `value`: The password for MQTT authentication.
- **Description**: Sets the MQTT password.

#### `bool publish(String topic, String value)`
- **Parameters**:
  - `topic`: The subtopic under the base topic (e.g., "status").
  - `value`: The message payload to publish.
- **Returns**: `true` if the message was sent successfully, `false` otherwise.
- **Description**: Publishes a message to the MQTT broker under the constructed topic (base topic + subtopic).

#### `void addReportStepHook(std::function<void(int)> func)`
- **Parameters**:
  - `func`: A callback function that accepts an integer step parameter.
- **Description**: Registers a callback function to report execution steps (e.g., errors or progress).

#### `void reportStep(int step)`
- **Parameters**:
  - `step`: The step value to report (e.g., `-1` for errors, `1` or `2` for progress).
- **Description**: Triggers all registered step-reporting callbacks with the given step value.

### Private Members
- `Logger* _logger`: Logger instance for event logging.
- `WiFiClient _espClient`: Underlying WiFi client for MQTT communication.
- `PubSubClient _client`: MQTT client instance.
- `String _topicPrefix`: Base topic prefix for messages (default: "IoT").
- `const char* _server`, `_clientId`, `_username`, `_password`: MQTT configuration parameters.
- `int _port`: MQTT broker port number.
- `std::vector<std::function<void(int)>> _reportStepsHooks`: List of registered step-reporting hooks.

---

## Usage Example

### Basic Setup
```cpp
#include <Logger/ConsoleLogger.h>
#include <MqttManager/MqttManager.h>

ConsoleLogger logger;
MqttManager mqtt(&logger);

void setup() {
  logger.begin();
  mqtt.setServer("mqtt.eclipse.org");
  mqtt.setPort(1883);
  mqtt.setTopicPrefix("EnergyMonitor", "Device1");
  mqtt.begin();
}

void loop() {
  mqtt.loop();
  if (mqtt.publish("status", "online")) {
    logger.log("Message published successfully.");
  } else {
    logger.log("Failed to publish message.");
  }
  delay(5000);
}s
```

### Adding Callbacks
```cpp
void onStepReport(int step) {
  if (step == -1) logger.logf("MQTT error (step: %d)", step);
}

void setup() {
  // ... (previous setup)
  mqtt.addReportStepHook(onStepReport);
}
```

## Integration with Logger Framework
The `MqttManager` class is designed to work with the `Logger` framework. By passing a `Logger` instance (e.g., `ConsoleLogger` or `TelnetLogger`) to the constructor, all MQTT-related events (e.g., connection status, errors) can be logged to the desired output.

