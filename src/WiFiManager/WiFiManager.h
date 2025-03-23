/**
 * @file WiFiManager.h
 * @brief Manages Wi-Fi connectivity and access points for the ESP8266.
 * 
 * This class handles Wi-Fi connections, access point creation, and reconnection attempts. 
 * It integrates with an HTTP server that provides API endpoints to retrieve nearby 
 * Wi-Fi access points. Additionally, when internet connectivity is available, it 
 * synchronizes and maintains the system date and time using the Network Time Protocol 
 * (NTP).
 */
#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include <vector>
#include "ConfigurationManager/ConfigurationManager.h"
#include "HTTPServerManager/HTTPServerManager.h"
#include "Logger/Logger.h"

/**
 * @brief Enumeration for different Wi-Fi operation modes.
 */
enum OperationMode {
    INIT,      ///< Initial state before connecting
    NORMAL,    ///< Connected to Wi-Fi and operating normally
    SETTINGS   ///< Configuration mode (AP mode)
};

class WiFiManager {
public:
    /**
     * @brief Constructs a WiFiManager object.
     * 
     * @param serverManager Reference to the HTTPServerManager instance.
     * @param logger Pointer to the Logger instance.
     */
    WiFiManager(HTTPServerManager& serverManager, Logger* logger = nullptr);

    /**
     * @brief Initializes the WiFiManager and connects to Wi-Fi.
     */
    void begin();
    
    /**
     * @brief Main loop for WiFiManager to handle reconnections and mode switching.
     */
    void loop();

    void setSSID(const char* value){            _SSID = value; };
    void setPassword(const char* value){    _password = value; };
    void setHostname(const char* value){    _hostname = value;}
    void setAPSSID(const char* value){        _apSSID = value;}      
    void setAPPassword(const char* value){_apPassword = value;}
    void setTimeServer(const char* value){_timeServer = value; };
    void setTimeZone(const char* value){    _timeZone = value; };

    /**
     * @brief Attempts to connect to a Wi-Fi access point for the first time.
     * 
     * @return True if successfully connected, false otherwise.
     */
    bool firstConnectToAP();

    /**
     * @brief Attempts to reconnect to the Wi-Fi access point if disconnected.
     * 
     * @return True if connected, false otherwise.
     */
    bool reConnecToAP();
    
    /**
     * @brief Connects to a Wi-Fi access point using stored credentials.
     * 
     * @return True if successfully connected, false otherwise.
     */
    bool connectToAP();   

    /**
     * @brief Creates a Wi-Fi access point.
     */ 
    void createAP();

    /**
     * @brief Reboots the ESP8266.
     */
    void reboot();
    
    /**
     * @brief Adds a function hook to report connection steps.
     * 
     * @param func Callback function for reporting steps.
     */
    void addReportStepHook(std::function<void(int)> func);
    
    /**
     * @brief Reports a connection step to all registered hooks.
     * 
     * @param step Step identifier.
     */
    void reportStep(int step);

    /**
     * @brief Registers HTTP endpoints for Wi-Fi management.
     */
    void registerEndpoints();
    
    /**
     * @brief Handles scanning for nearby Wi-Fi access points.
     * 
     * @param server Reference to the ESP8266WebServer instance.
     */
    void handleScanAPs(ESP8266WebServer& server);

private:    
    HTTPServerManager& _serverManager;  ///< Reference to the HTTP server manager.
    Logger* _logger;                    ///< Pointer to the logger.

    const char* _SSID;                  ///< Wi-Fi SSID.
    const char* _password;              ///< Wi-Fi password.
    const char* _hostname;              ///< Device hostname.

    // In case the microcontroller is to act as Access Point (AP), below are attributes to hold credentials
    const char* _apSSID;                ///< AP mode SSID.
    const char* _apPassword;            ///< AP mode password.
    bool _APstarted = false;            ///< Indicates if AP mode is active.

    const char* _timeServer="pool.ntp.org"; ///< Time server, default = pool.ntp.org.
    const char* _timeZone="";           ///< Time zone, as defined in https://github.com/esp8266/Arduino/blob/master/cores/esp8266/TZ.h

    OperationMode _operationMode;       ///< Current operation mode.
    unsigned long _lastTime = 0;        ///< Time tracking variable.

    std::vector<std::function<void(int)>> _reportStepsHooks;  ///< List of hooks.
};

#endif