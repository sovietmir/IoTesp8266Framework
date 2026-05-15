# Change Log
## [1.3.0] - 2026-05-15
New methods:
- `getFileSize()`: New method to get the size of a file in bytes.
- `isRotationNeeded()`: Checks whether the current log file has reached or exceeded the threshold.
- `getTotalLogSize()`: Helper method to calculate the total storage used by logs for a specific prefix.
- `checkAndRotate()`: Convenience method that checks whether rotation is needed and performs it if necessary.
- `enableSystemLoggingToFS()`: Enables or disables logging of system informational messages to the file system.
- `isSystemLoggingToFS()`: Returns the current state of system informational logging to the file system.
- `maxArchives()`: Sets and gets the maximum number of log archives to keep in the file system.
- `logSizeThreshold()`: Sets and gets the file size threshold for log rotation.

New private attributes: `_do_log_system_to_FS`, `_max_archives`, and `_log_size_threshold`.

Updated `rotateLogs()`: Modified to properly handle rotation by replacing `MAX_ARCHIVES` with `maxArchives()`.

Updated `logSystem()`: Now checks `isSystemLoggingToFS()`. If it returns `true`, logs are written to the file system; otherwise, they are written to the console.

Added `.gitattributes` to the project.

## [1.2.1] - 2026-05-12
- Enhanced the `Logger` interface with file system logging:
    - Added methods: `logToFS`, `logWithLevel`, `logError`, and `logSystem`
    - Added log rotation methods: `moveFile` and `rotateLogs`
- Updated the `WiFiManager` class to log errors to the file system
- Added a `detailed` option to `WiFiManager::handleScanAPs`, which returns detailed information about nearby access points (APs)

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
