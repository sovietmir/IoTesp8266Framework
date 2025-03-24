# Logger Library Framework

This library provides a flexible logging framework for Arduino and ESP8266 projects. It supports multiple logging outputs, including console (Serial) and Telnet, and allows for easy extension to other logging mechanisms.

## Table of Contents
1. [Overview](#overview)
2. [Classes](#classes)
   - [Logger](#logger)
   - [ConsoleLogger](#consolelogger)
   - [TelnetLogger](#telnetlogger)
3. [Usage](#usage)
4. [Examples](#examples)
5. [Extending the Framework](#extending-the-framework)

## Overview

The Logger library is designed to provide a unified interface for logging messages in various environments. It supports logging to the Serial console and Telnet clients, and it can be extended to support other logging mechanisms.

## Classes

### Logger

The `Logger` class is an abstract base class that defines the interface for logging. It provides methods for initializing the logger, logging messages, and formatting log messages with timestamps.

#### Methods
- `void begin()`: Initializes the logger. Default implementation does nothing.
- `void loop()`: Handles any ongoing tasks for the logger. Default implementation does nothing.
- `void log(const char* message)`: Logs a message. This is a pure virtual method and must be implemented by derived classes.
- `void log(String message)`: Logs a message provided as a `String` object. Default implementation converts the `String` to a C-string and calls `log(const char*)`.
- `void logf(const char* format, Args... args)`: Logs a formatted message. This is a variadic template method that formats the message using `snprintf` and then logs it.
- `static String timeToString()`: Returns the current time as a formatted string.

### ConsoleLogger

The `ConsoleLogger` class logs messages to the Serial console.

#### Methods
- `void begin()`: Initializes the Serial communication.
- `void log(const char* message)`: Logs a message to the Serial console.

### TelnetLogger

The `TelnetLogger` class logs messages to a Telnet client. It also forwards Serial data to the Telnet client and vice versa.

#### Methods
- `TelnetLogger(uint16_t port = 23)`: Constructor that initializes the Telnet server on the specified port (default is 23).
- `void begin()`: Initializes the Telnet server and Serial communication.
- `void loop()`: Handles Telnet client connections and data forwarding.
- `void log(const char* message)`: Logs a message to both the Serial console and the Telnet client.

## Usage

To use the Logger library, include the appropriate header files and create instances of the desired logger classes. Initialize the logger using the `begin()` method and log messages using the `log()` or `logf()` methods.

### Example: Using ConsoleLogger

```cpp
#include <Logger/ConsoleLogger.h>

ConsoleLogger logger;

void setup() {
    logger.begin(); ///< This call is essential `Serial.begin(115200);`
    logger.log("Hello, Serial!");
}

void loop() {
    // Your code here
}
```

### Example: Using TelnetLogger

```cpp
#include <Logger/TelnetLogger.h>

TelnetLogger logger;

void setup() {
    logger.begin();
    logger.log("Hello, Telnet!");
}

void loop() {
    logger.loop();
    // Your code here
}
```

## Extending the Framework

To extend the framework with a new logging mechanism, create a new class that inherits from `Logger` and implement the `log(const char*)` method. Optionally, override the `begin()` and `loop()` methods if initialization or ongoing tasks are required.

### Example: Custom Logger

```cpp
#include <Logger/Logger.h>

class CustomLogger : public Logger {
public:
    void begin() override {
        // Initialize custom logging mechanism
    }

    void log(const char* message) override {
        // Log message using custom mechanism
    }
};
```


