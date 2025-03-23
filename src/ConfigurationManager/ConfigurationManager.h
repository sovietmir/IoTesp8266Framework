#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <ArduinoJson.h>
#include <LittleFS.h>
#include "HTTPServerManager/HTTPServerManager.h"
#include "Logger/Logger.h"

/**
 * @class ConfigurationManager
 * @brief Manages configuration data, including loading, saving, and retrieving configuration values.
 * 
 * This class provides methods to interact with configuration data stored in a JSON format. 
 * It supports reading and writing configuration files, as well as handling HTTP API calls 
 * for configuration management.
 */
class ConfigurationManager {
public:
    /**
     * @brief Constructs a new ConfigurationManager object.
     * 
     * @param name The name of the configuration file.
     * @param serverManager Reference to the HTTPServerManager for registering endpoints.
     * @param logger Pointer to the Logger for logging messages.
     */
    ConfigurationManager(const char* name, HTTPServerManager& serverManager, Logger* logger);

    /**
     * @brief Loads the configuration from the file system.
     * 
     * @return true if the configuration was successfully loaded, false otherwise.
     */
    bool loadConfig();

    /**
     * @brief Saves the current configuration to the file system.
     * 
     * @return true if the configuration was successfully saved, false otherwise.
     */
    bool saveConfig();

    /**
     * @brief Resets the current configuration to an empty state.
     */
    void resetConfig();

    /**
     * @brief Retrieves a string value from the configuration.
     * 
     * @param path The path to the configuration value (e.g., "wifi.ssid").
     * @param defaultValue The default value to return if the path is not found.
     * @return The configuration value as a const char*, or the default value if not found.
     */
    const char* getValue(const char* path, const char* defaultValue);

    /**
     * @brief Retrieves an integer value from the configuration.
     * 
     * @param path The path to the configuration value (e.g., "mqtt.port").
     * @param defaultValue The default value to return if the path is not found.
     * @return The configuration value as an int, or the default value if not found.
     */
    int getValue(const char* path, int defaultValue);

    /**
     * @brief Sets the current configuration to the provided JSON object.
     * 
     * @param newConfig The new configuration as a JsonObject.
     */
    void setConfig(const JsonObject& newConfig);

    /**
     * @brief Retrieves the current configuration as a JsonDocument.
     * 
     * @return The current configuration as a JsonDocument.
     */
    JsonDocument getConfig();

    /**
     * @brief Initializes the ConfigurationManager and registers HTTP endpoints.
     */
    void begin();

    /**
     * @brief Registers HTTP endpoints for configuration management.
     */
    void registerEndpoints();

    /**
     * @brief Handles HTTP POST requests for updating the configuration.
     * 
     * @param server Reference to the ESP8266WebServer handling the request.
     */
    void handleConfigPost(ESP8266WebServer& server);

    /**
     * @brief Handles HTTP GET requests for retrieving the current configuration.
     * 
     * @param server Reference to the ESP8266WebServer handling the request.
     */
    void handleGetCurrentConfig(ESP8266WebServer& server);

private:
    const char* _name;               ///< The name of the configuration file.
    HTTPServerManager& _serverManager; ///< Reference to the HTTPServerManager.
    Logger* _logger;                 ///< Pointer to the Logger for logging messages.
    JsonDocument _config;            ///< The current configuration data.
};

#endif