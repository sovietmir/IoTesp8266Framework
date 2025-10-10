# Change Log
## [1.2.0] - 2025-10-10
Added `Sensor<T>` class — a **generic base class** for sensor monitors in ESP8266 projects. It provides the logic for:
    - Periodic reading of sensor metrics
    - Logging
    - Broadcasting over WebSocket
    - Serving REST API endpoints
    - Attaching hooks for reporting steps and metrics 


## [1.1.1] - 2025-03-24
### Added
Added documentation for each component

## [1.1.0] - 2025-03-24
### Fix
Method begin() of classes of interface `Logger` is enriched with `Serial.begin()`. Methods affected: 
 - ConsoleLogger::begin()
 - TelnetLogger::begin()
 
### Modified 
- A comment in Logger.h
- Some rephrasing in README.md




## [1.0.0] - 2025-03-23
### Added
- Initial release of the project.
