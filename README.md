# IoTesp8266Framework

A comprehensive framework for building IoT applications on the ESP8266 microcontroller. This library provides functionality for handling common IoT tasks such as WiFi connectivity, OTA updates, MQTT communication, and more. 

## Features
- **HTTP Server**: Serve static files (e.g., HTML, CSS, JS) and handle HTTP requests (GET, POST, etc.). Built-in WebSocket integration. Allows integration of additional endpoints.
- **Logger**: Abstract logging for debugging.
- **OTA**: Over-The-Air updates for the ESP8266.
- **WiFi**: Manage WiFi connections.
- **MQTT**: Publish and subscribe to MQTT topics.
- **Configuration**:  Manage configuration files in JSON format, with support for reading, writing, and HTTP API access..


## Installation
### Manual Installation
1. Download the library as a ZIP file or clone the repository into a directory:
   ```sh
   git clone https://github.com/sovietmir/IoTesp8266Framework.git
   ```
2. Add the library to your PlatformIO project by placing it in the `lib/` folder or specifying the local path (adding it as a dependency) in `platformio.ini`:
   ```ini
   lib_deps =
       	file:///path/to/IoTesp8266Framework
   ```
### PlatformIO Dependency
1. Add the library to your PlatformIO project by adding it as a dependency in `platformio.ini`:
   ```ini
   lib_deps =
       https://github.com/sovietmir/IoTesp8266Framework.git
   ```

## Filesystem Setup
To use features like `HTTPServerManager`, `OTA`, and `ConfigurationManager`, you need to build and upload a filesystem image to the microcontroller. This filesystem contains static files (e.g., HTML, CSS) and an example of configuration file that could be used by the framework.

1. Copy the `data/` folder from the framework into your project directory.
2. Configure PlatformIO to use LittleFS by adding the following to `platformio.ini`:
   ```ini
   build_flags = -DPIO_FRAMEWORK_ARDUINO_LITTLEFS -I include
   board_build.filesystem = littlefs
   board_build.ldscript = eagle.flash.4m2m.ld
   ```
3. Build the filesystem image:
   ```sh
   pio run --target buildfs 
   ```
   The buildfs target compiles and packages the files from your project's data directory into a filesystem image that can be uploaded to the microcontroller. The output of the above command is a .bin file (`littlefs.bin`), which contains the filesystem image. This file can now be uploaded to the microcontroller's flash memory.
4. Upload the filesystem image to the microcontroller:
   ```sh
   pio run --target uploadfs
   ```
   Note: Ensure the microcontroller is in flash mode (booted with `GPIO0` grounded) during the upload process.


## Usage
In your project include the main library header, that includes all the class headers:
```cpp
#include <IoTesp8266Framework.h>
```
or include individual modules:
```cpp
#include <ConfigurationManager/ConfigurationManager.h>
#include <HTTPServerManager/HTTPServerManager.h>
#include <Logger/TelnetLogger.h>
#include <OTA/OTA.h>
#include <WiFiManager/WiFiManager.h>
#include <MqttManager/MqttManager.h>
```

## Component Documentation
| Component | Description | Documentation |
|-----------|-------------|---------------|
| `HTTPServerManager` | HTTP server with static file serving | [View](documentation/HTTPServerManager.md) |
| `Logger` | Unified logging interface | [View](documentation/Logger.md) |
| `OTA` | Over-the-air updates | [View](documentation/OTA.md) |
| `WiFiManager` | Dual-mode WiFi management | [View](documentation/WiFiManager.md) |
| `MqttManager` | MQTT client wrapper | [View](documentation/MqttManager.md) |
| `ConfigurationManager` | JSON config management | [View](documentation/ConfigurationManager.md) |


## Structure 

```
IoTesp8266Framework/
├── src/
│   ├── HTTPServerManager/      # [Docs](documentation/HTTPServerManager.md)
│   ├── Logger/                 # [Docs](documentation/Logger.md)
│   ├── OTA/                    # [Docs](documentation/OTA.md)
│   ├── WiFiManager/            # [Docs](documentation/WiFiManager.md)
│   ├── MqttManager/            # [Docs](documentation/MqttManager.md)
│   └── ConfigurationManager/   # [Docs](documentation/ConfigurationManager.md)
├── data/                       # Static files and configs
├── documentation/              # Component documentation
├── library.json
├── CHANGELOG.json
└── README.md
```
---
