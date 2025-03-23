/**
 * @file OTA.h
 * @brief Over-the-Air (OTA) update and file management class for ESP8266.
 *
 * This class provides OTA firmware updates, file upload/download, 
 * and filesystem management functionalities through an HTTP API.
 */

#ifndef OTA_H
#define OTA_H

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <Updater.h>
#include "HTTPServerManager/HTTPServerManager.h"
#include "Logger/Logger.h"

/**
 * @class OTA
 * @brief Handles OTA firmware updates and file system operations over HTTP.
 */
class OTA {
public:
    /**
     * @brief Constructs an OTA object.
     * @param serverManager Reference to the HTTPServerManager handling HTTP requests.
     * @param logger Pointer to the Logger for logging messages.
     */
    OTA(HTTPServerManager& serverManager, Logger* logger = nullptr);

    /**
     * @brief Initializes the OTA service.
     * 
     * Registers API endpoints for firmware updates and file system operations.
     */
    void begin();

    /**
     * @brief Registers all OTA-related HTTP endpoints.
     */
    void registerEndpoints();

    /**
     * @brief Adds a callback hook for reporting OTA update steps.
     * @param func Callback function receiving an integer step indicator.
     */
    void addReportStepHook(std::function<void(int)> func);

    /**
     * @brief Reports an OTA update step to registered hooks.
     * @param step The current step of the OTA update process.
     */
    void reportStep(int step);

private:
    HTTPServerManager& _serverManager; ///< Reference to the server manager.
    Logger* _logger; ///< Pointer to the logger.

    std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of OTA update step hooks.
    
    void handleFirmwareUpload(ESP8266WebServer& server);

    void handleFileUpload(ESP8266WebServer& server);

    void handleDirectoryList(ESP8266WebServer& server);

    void handleFileSystemRequest(ESP8266WebServer& server);

    void handleDownloadRequest(ESP8266WebServer& server);

    void handleDeleteRequest(ESP8266WebServer& server);

    void handleAddDirectoryRequest(ESP8266WebServer& server);

    void listFilesRecursive(const String& dirPath, JsonArray& filesArray);
    
};

#endif // OTA_H
