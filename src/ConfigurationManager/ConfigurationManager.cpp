#include "ConfigurationManager.h"

ConfigurationManager::ConfigurationManager(const char* name, HTTPServerManager& serverManager, Logger* logger)
        : _name(name), 
          _serverManager(serverManager),
          _logger(logger) 
          {}

bool ConfigurationManager::loadConfig() {
    if (!LittleFS.begin()) {
        return false;
    }

    File configFile = LittleFS.open("/"+String(_name)+".json", "r");
    if (!configFile) {
        return false;
    }

    DeserializationError error = deserializeJson(_config, configFile);
    configFile.close();

    return !error;
}

bool ConfigurationManager::saveConfig() {
    File configFile = LittleFS.open("/"+String(_name)+".json", "w");
    if (!configFile) {
        return false;
    }

    serializeJson(_config, configFile);
    configFile.close();
    return true;
}

void ConfigurationManager::resetConfig() {
    _config.clear();
}




JsonVariant resolvePath(JsonVariant root, const char* path) {
    JsonVariant current = root;
    char buffer[128];
    strncpy(buffer, path, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    char* token = strtok(buffer, ".");
    while (token != nullptr) {
        if (!current.is<JsonObject>()) {
            return JsonVariant(); // Return null variant if the path is invalid
        }
        current = current[token];
        token = strtok(nullptr, ".");
    }
    return current;
}
/**
 * Usage example: const char* ssid = getValue("wifi.ssid", "");
*/
const char* ConfigurationManager::getValue(const char* path, const char* defaultValue) {
    JsonVariant value = resolvePath(_config, path);
    if (value.is<const char*>()) {
        return value.as<const char*>();
    }
    return defaultValue;    
}
/**
 * Usage example: int mqttPort = getValue("mqtt.port", 1883);
*/
int ConfigurationManager::getValue(const char* path, int defaultValue) {
    JsonVariant value = resolvePath(_config, path);
    if (value.is<int>()) {
        return value.as<int>();
    }
    return defaultValue;
}



void ConfigurationManager::setConfig(const JsonObject& newConfig) {
    _config.clear();
    _config.set(newConfig);
}

JsonDocument ConfigurationManager::getConfig() {
    return _config;
}


void ConfigurationManager::begin() {
  registerEndpoints();
}
void ConfigurationManager::registerEndpoints() {
    // Register configuration backend calls handling

    _serverManager.registerPage("/api/"+String(_name)+"/read", HTTP_GET, [this](ESP8266WebServer& server) { handleGetCurrentConfig(server); });

    // Handle POST request for configurations
    _serverManager.registerPage("/api/"+String(_name)+"/save", HTTP_POST, [this](ESP8266WebServer& server) { handleConfigPost(server); });
}

void ConfigurationManager::handleGetCurrentConfig(ESP8266WebServer& server) {
    String configJson = "";
    serializeJson(getConfig(), configJson);
    server.send(200, "application/json", configJson);
}

void ConfigurationManager::handleConfigPost(ESP8266WebServer& server) {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\": \"nok1\", \"error\":\"Bad Request\"}");
        return;
    }

    String json = server.arg("plain");
        // Serialize to a String in pretty format
    //String output;
    //serializeJsonPretty(json, output);

    // Print the pretty JSON string
    if (_logger != nullptr) _logger->log(json+"\n");
    JsonDocument newConfig;
    DeserializationError error = deserializeJson(newConfig, json);
    if(error) {
        server.send(500, "application/json", "{\"status\": \"nok2\", \"error\":\"Failed to deserializeJson the request data\"}");
        return;
    }
    setConfig(newConfig.as<JsonObject>());

    if (!saveConfig()) {
        server.send(500, "application/json", "{\"status\": \"nok3\", \"error\":\"Failed to save configuration\"}");
        return;
    }

    server.send(200, "application/json", "{\"status\": \"ok\", \"message\":\"Configuration saved successfully\"}");
    if (_logger != nullptr) _logger->log("Configuration updated via HTTP POST.\n");
}


