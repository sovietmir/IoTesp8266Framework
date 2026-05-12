#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <time.h>  // for time() ctime()

const int MAX_ARCHIVES = 3;

class Logger {
public:
    virtual void begin() {
      if (!LittleFS.begin()) {
        return;
      }
    };
    virtual void loop() {};
    virtual void log(const char* message) = 0;
    virtual void log(String message) {
        log(message.c_str());
    }

    // This is variadic template. Usage example: logger.logf("Date Now is %s, Timestamp is %ld", "2025-01-13T12:34:56Z", timestamp);
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

    bool moveFile(const char* oldPath, const char* newPath) {
      bool success = true;
    // Remove destination first if it exists
      if (LittleFS.exists(oldPath)) {
        if (LittleFS.exists(newPath)) {
          LittleFS.remove(newPath);
        }
      // rename() is effectively the "move" operation
        success = LittleFS.rename(oldPath, newPath);
        if(!success) {
          logf("Failed to move: %s -> %s\n", oldPath, newPath);
        }
      }
      return success;  
    }

    /**
     * Rotates log files to maintain a limited archive history
     * 
     * Creates numbered backups of the current log file, shifting existing backups
     * by one index. The most recent log becomes .1, previous .1 becomes .2, etc.
     * 
     * @param prefix Log file prefix (e.g., "system" → system.log, system.1.log, system.2.log...)
     * @param path   Directory path where log files are stored (default: "/logs/")
     * 
     * @example With MAX_ARCHIVES=3:
     *          Before: system.log, system.1.log, system.2.log
     *          After:  system.log (fresh), system.1.log (old system.log), 
     *                  system.2.log (old system.1.log), system.3.log (old system.2.log)
     * 
     * @note The current active log (system.log) is not rotated itself - this function
     *       merely shifts existing archives to make room. Call logToFS() afterward
     *       to write to the fresh system.log.
     */
    void rotateLogs(String prefix="system", String path="/logs/") {
      for (int i = MAX_ARCHIVES; i > 0; i--) {
        String oldName = path + prefix + ((i>1)?"."+String(i - 1):"") + ".log"; //< /logs/system.<i-1>.log
        String newName = path + prefix +"." + String(i) + ".log"; //< /logs/system.<i>.log
        moveFile(oldName.c_str(), newName.c_str());        
      }
    }

    void logToFS(String message, String prefix="system", String path="/logs/") {
      digitalWrite(LED_BUILTIN, HIGH);

      log(message+"\n");
      logf("Start writing to file [%d]\n", millis());
      File logFile = LittleFS.open( path + prefix + ".log", "a");
      if (!logFile){
        digitalWrite(LED_BUILTIN, LOW);
        return;
      }

      logFile.print("["+timeToString()+"]");

      float seconds = millis() / 1000;
      logFile.printf(" [%.3fs] ", seconds);
      logFile.println(message);
      logFile.close();
      logf("End writing to file [%.d]\n", millis());
      digitalWrite(LED_BUILTIN, LOW);
    }


    /**
     * Logs a formatted message to both console or telent and persistent storage
     * 
     * The message is written to:
     *   - Console or/and telent serial(via log())
     *   - File: /logs/{level}.log (via logToFS())
     * 
     * @param level  Log severity/type - used as the log filename prefix
     * @param format printf-style format string (supports %s, %d, %f, etc.)
     * @param args   Variable arguments matching the format specifiers
     * 
     * @example logWithLevel("error", "Sensor %d failed with code %d", sensorId, errorCode);
     *          // Writes to console and /logs/error.log
     */
    template <typename... Args>
    void logWithLevel(const char* level, const char* format, Args... args) {
      char buffer[128];
      snprintf(buffer, sizeof(buffer), format, args...);
      logToFS(String(buffer), String(level));  
    }
    // Overload for simple String messages (preserves original interface)
    void logWithLevel(const char* level, String message) {
      logToFS(message, String(level));
    }

   // This is variadic template. Usage example: logger.logError("Reset reason: %s", ESP.getResetReason().c_str());
    template <typename... Args>
    void logError(const char* format, Args... args) { 
      logWithLevel("error", format, args...); 
    }
    void logError(String message) {
      logWithLevel("error", message);
    }

    template <typename... Args>
    void logSystem(const char* format, Args... args) { 
      logWithLevel("system", format, args...); 
    }
    void logSystem(String message) {
      logWithLevel("error", message);
    }

};

#endif