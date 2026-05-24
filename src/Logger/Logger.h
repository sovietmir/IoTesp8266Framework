#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <time.h>  // for time() ctime()


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

    /**
     * Gets the size of a file
     * 
     * @param path Path to the file
     * @return Size of the file in bytes, or 0 if file doesn't exist
     */
    static size_t getFileSize(const char* path) {
      if (!LittleFS.exists(path)) {
        return 0;
      }
      
      File file = LittleFS.open(path, "r");
      if (!file) {
        return 0;
      }
      
      size_t size = file.size();
      file.close();
      return size;
    }

    /**
     * Checks if log rotation is needed based on file size
     * 
     * @param prefix Log file prefix (e.g., "system" → system.log)
     * @param path   Directory path where log files are stored (default: "/logs/")
     * @return true if rotation is needed, false otherwise
     */
    bool isRotationNeeded(String prefix = "system", String path = "/logs/") {
      String currentLogPath = path + prefix + ".log";
      size_t currentSize = getFileSize(currentLogPath.c_str());
      return currentSize >= logSizeThreshold();
    }

    /**
     * Calculate total storage used by logs for a specific prefix
     * 
     * @param prefix Log file prefix
     * @param path   Directory path where log files are stored
     * @return Total size in bytes
     */
    size_t getTotalLogSize(String prefix = "system", String path = "/logs/") {
      size_t totalSize = 0;
      
      // Current log
      totalSize += getFileSize((path + prefix + ".log").c_str());
      
      // Archives
      for (size_t i = 1; i <= maxArchives(); i++) {
        totalSize += getFileSize((path + prefix + "." + String(i) + ".log").c_str());
      }
      
      return totalSize;
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
     * @example With maximum archives to keep=3:
     *          Before: system.log, system.1.log, system.2.log
     *          After:  system.log (fresh), system.1.log (old system.log), 
     *                  system.2.log (old system.1.log), system.3.log (old system.2.log)
     * 
     * @note The current active log (system.log) is not rotated itself - this function
     *       merely shifts existing archives to make room. Call logToFS() afterward
     *       to write to the fresh system.log.
     */
    void rotateLogs(String prefix="system", String path="/logs/") {
      for (size_t i = maxArchives(); i > 0; i--) {
        String oldName = path + prefix + ((i>1)?"."+String(i - 1):"") + ".log"; //< /logs/system.<i-1>.log
        String newName = path + prefix +"." + String(i) + ".log"; //< /logs/system.<i>.log
        moveFile(oldName.c_str(), newName.c_str());        
      }
    }

    /**
     * Checks if rotation is needed and performs it if necessary
     * Call this before writing to a log file
     * 
     * @param prefix Log file prefix
     * @param path   Directory path where log files are stored
     * @return true if rotation was performed, false otherwise
     */
    bool checkAndRotate(String prefix = "system", String path = "/logs/") {
      if (isRotationNeeded(prefix, path)) {
        logf("Rotation triggered for %s (size: %d bytes, threshold: %d bytes)\n", 
             prefix.c_str(), getFileSize((path + prefix + ".log").c_str()), logSizeThreshold());
        rotateLogs(prefix, path);
        return true;
      }
      return false;
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
      uint32_t now = millis();        
      if (_lastErrorLogRotationCheckMs==0 || (now - _lastErrorLogRotationCheckMs >= (rotationCheckIntervalSec() * 1000))) {
        _lastErrorLogRotationCheckMs = now;
        checkAndRotate();
      }
      logWithLevel("error", format, args...); 
    }
    void logError(String message) {
      logError(message.c_str());
    }

    template <typename... Args>
    void logSystem(const char* format, Args... args) { 
      if(isSystemLoggingToFS()) {
        uint32_t now = millis();        
        if (_lastSystemLogRotationCheckMs==0 || (now - _lastSystemLogRotationCheckMs >= (rotationCheckIntervalSec() * 1000))) {
          _lastSystemLogRotationCheckMs = now;
          checkAndRotate();
        }

        logWithLevel("system", format, args...); 
      }
      else {
        logf(format, args...);
        log(String(" [")+millis()+"]\n");
      }
    }
    void logSystem(String message) {
      logSystem(message.c_str());
    }


  /**
   * Enable or disable logging of system informational messages to the file system
   * 
   * When enabled, system messages are written to both:
   *   - Console/telnet (via log())
   *   - File system (/logs/system.log)
   * 
   * When disabled, system messages are only written to console/telnet.
   * This helps reduce flash memory wear during high-frequency system logging.
   * Additionally, it reduces initialization time, as classes such as WifiManager
   * typically write system logs when connecting to an access point.
   * 
   * @param value true to enable file system logging, false to disable
   * @return The new state (true = enabled, false = disabled)
   * 
   * @note Error messages are always logged to file system regardless of this setting
   * @see logSystem()
   */
    bool enableSystemLoggingToFS(bool enable){
      _do_log_system_to_FS=enable;
      log(_do_log_system_to_FS ? 
        "System logging to file system enabled\n" : 
        "System logging to file system disabled\n");
      return _do_log_system_to_FS;
    }
    /**
     * Get the current state of system informational logging to file system
     * 
     * @return true if system messages are being logged to file system, false if only console
     * 
     * @see enableSystemLoggingToFS(bool) to change this setting
     * @see logSystem() which uses this setting
     */
    bool isSystemLoggingToFS() const {
      return _do_log_system_to_FS;
    }
   
   /**
     * Set the maximum number of log archives to keep in the filesystem when 
     * rotating the log files
     * 
     * @param max maximum number of log archives
     */
    size_t maxArchives(size_t max) {
      _max_archives = max;
      logf("Log size threshold set to %d bytes\n", _max_archives);
      return _max_archives;
    }    
    /**
     * Get the maximum number of log archives to keep
     * 
     * @return maximum number of log archives
     */
    size_t maxArchives() const {
      return _max_archives;
    }

   /**
     * Set the file size threshold for log rotation
     * 
     * @param threshold Size in bytes (e.g., 100*1024 for 100KB)
     */
    size_t logSizeThreshold(size_t threshold) {
      _log_size_threshold = threshold;
      logf("Log size threshold set to %d bytes\n", _log_size_threshold);
      return _log_size_threshold;
    }    
    /**
     * Get the current file size threshold
     * 
     * @return Size threshold in bytes
     */
    size_t logSizeThreshold() const {
      return _log_size_threshold;
    }

    /**
     * Set the interval between automatic log rotation checks.
     * 
     * When logging to a file (methods `logSystem`, `logError`), the system 
     * checks if the log file has exceeded the size threshold and needs rotation 
     * (by calling  `checkAndRotate()`). This setting controls how often that 
     * check occurs to avoid excessive file system access.
     * 
     * @param value Interval in seconds
     * @return The new interval value
    */
    uint32_t rotationCheckIntervalSec(uint32_t value) {
      _rotationCheckIntervalSec = value;
      return _rotationCheckIntervalSec;
    }
    /**
     * Get the current rotation log check interval.
     * 
     * @return Interval in seconds
     */
    uint32_t rotationCheckIntervalSec() const {
      return _rotationCheckIntervalSec;
    }
private:
  bool _do_log_system_to_FS  = false; //< The current state of system informational logging to file system 
  size_t _max_archives       = 3;     //< The maximum number of log archives to keep in the filesystem when rotating the log files
  size_t _log_size_threshold = 100 * 1024; //< Size threshold for rotation in bytes

  uint32_t _rotationCheckIntervalSec = 600;  // Check every 10 minuntess
  uint32_t _lastErrorLogRotationCheckMs  = 0;
  uint32_t _lastSystemLogRotationCheckMs  = 0;

};

#endif