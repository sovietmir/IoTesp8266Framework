#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <time.h>  // for time() ctime()

class Logger {
public:
    virtual void begin() {};
    virtual void loop() {};
    virtual void log(const char* message) = 0;
    virtual void log(String message) {
        log(message.c_str());
    }

    // New logf method. This is variadic template. Usage example: logger.logf("Date Now is %s, Timestamp is %ld", "2025-01-13T12:34:56Z", timestamp);
    template <typename... Args>
    void logf(const char* format, Args... args) {
        char buffer[128]; // Adjust size as needed
        snprintf(buffer, sizeof(buffer), format, args...);
        log(buffer);
    }

    static String timeToString() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            return String("");
        }
        
        char timeString[40];  // Safe buffer size
        snprintf(timeString, sizeof(timeString), "%04d-%02d-%02d %02d:%02d:%02d", 
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, 
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        return String(timeString); 
    }
};

#endif