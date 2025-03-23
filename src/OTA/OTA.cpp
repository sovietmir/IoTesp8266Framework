#include "OTA.h"

OTA::OTA(HTTPServerManager& serverManager, Logger* logger) : _serverManager(serverManager),_logger(logger) {}

void OTA::begin(){
    registerEndpoints();
}

void OTA::registerEndpoints() {
    // Register firmware upload endpoint
    _serverManager.registerPage(
        "/api/firmware", 
        HTTP_POST, 
        [this](ESP8266WebServer& server) {
          handleFirmwareUpload(server);
        }, 
        [this](ESP8266WebServer& server) {
          handleFirmwareUpload(server); // Proper file upload handler
        }
    );

    // Register file upload endpoint
    _serverManager.registerPage(
        "/api/upload", 
        HTTP_POST, 
        [this](ESP8266WebServer& server) {
          handleFileUpload(server);
        },
        [this](ESP8266WebServer& server) {
          handleFileUpload(server); // Proper file upload handler
        }
    );
    
    _serverManager.registerPage("/api/reboot", HTTP_GET, [this](ESP8266WebServer& server) {
        if (_logger != nullptr) _logger->log("URI: /api/");
        server.send(200, "application/json", "{\"status\": \"ok\", \"message\": \"Microcontroller shall reboot in half a second.\"}"); 
        delay(500);
        ESP.restart();
    });

    // Register directory listing
    _serverManager.registerPage("/api/directories", HTTP_GET, [this](ESP8266WebServer& server) {
        if (_logger != nullptr) _logger->log("URI: /api/directories");
        handleDirectoryList(server);
    });


    // Register API endpoints
    _serverManager.registerPage("/api/files", HTTP_GET, [this](ESP8266WebServer& server) {
        handleFileSystemRequest(server);
    });

    _serverManager.registerPage("/api/download", HTTP_GET, [this](ESP8266WebServer& server) {
        handleDownloadRequest(server);
    });

    _serverManager.registerPage("/api/delete", HTTP_DELETE, [this](ESP8266WebServer& server) {
        handleDeleteRequest(server);
    });

    _serverManager.registerPage("/api/addDirectory", HTTP_POST, [this](ESP8266WebServer& server) {
        handleAddDirectoryRequest(server);
    });
}

void OTA::addReportStepHook(std::function<void(int)> func) {
  _reportStepsHooks.push_back(func);
}
void OTA::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}

/**
 * @brief Handles firmware upload via HTTP POST request.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleFirmwareUpload(ESP8266WebServer& server) {
    HTTPUpload& upload = server.upload();
    uint32_t update_size = ((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000);
     
    reportStep(1);
    //Update.runAsync(true);
    if (upload.status == UPLOAD_FILE_START) {
        if (_logger != nullptr) _logger->log("Start firmware update: "+upload.filename+".\n");
       if (!Update.begin(update_size)) { // Begin OTA process
            reportStep(2);
            reportStep(-3);
            Update.printError(Serial);
            server.send(500, "application/json", "{\"status\": \"nok1\", \"error\":\"Firmware update failed to start.\"}");
            if (_logger != nullptr) _logger->log("Firmware update failed to start.\n");
            return;
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { // End OTA process
            server.send(200, "application/json", "{\"status\": \"ok\", \"message\":\"Firmware updated successfully. Rebooting...\"}");
            if (_logger != nullptr) _logger->log("Firmware update successful. Rebooting...\n");
            delay(500);
            ESP.restart();
        } else {
            reportStep(-3);
            Update.printError(Serial);
            server.send(500, "application/json", "{\"status\": \"nok2\", \"error\":\"Firmware update failed.\"}");
            if (_logger != nullptr) _logger->log("Firmware update failed.\\n");
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        reportStep(2);
        reportStep(-3);
        Update.end();
        server.send(500, "application/json", "{\"status\": \"nok3\", \"error\":\"Firmware update aborted.\"}");
        if (_logger != nullptr) _logger->log("Firmware update aborted.\\n");
    }
}

/**
 * @brief Handles file upload via HTTP POST request.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleFileUpload(ESP8266WebServer& server) {
    HTTPUpload& upload = server.upload();
    static File file;         // Keeps file open across status changes
    static String directory;  // Keeps directory path across status changes
    
     reportStep(1);
    if (upload.status == UPLOAD_FILE_START) {
        if (_logger != nullptr) _logger->log("Start file upload: "+upload.filename+";");
        directory = server.arg("directory"); // Get the directory from the form
        if (directory.isEmpty()) directory = "/"; // Default to root if not provided

        String path = directory  + "/" + upload.filename; // Add subdirectory here
        file = LittleFS.open(path, "w");
        if (!file) {
            server.send(500, "application/json", "{\"status\": \"nok1\", \"error\":\"Failed to open file for writing.\"}");
            if (_logger != nullptr) _logger->log(" Failed to open file '"+path+"' for writing.\n");
            reportStep(2);
            reportStep(-3);
            return;
        }
        if (_logger != nullptr) _logger->log("\n");
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_logger != nullptr) _logger->log("File uploading (writing)...\n");        
        if (file) {
            file.write(upload.buf, upload.currentSize);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (file) {
            file.close();            
            server.send(200, "application/json", "{\"status\": \"ok\", \"message\":\"File uploaded successfully.\"}");
            if (_logger != nullptr) _logger->log("File upload successful.\n");
        } else {
            server.send(500, "application/json", "{\"status\": \"nok2\", \"error\":\"Failed to save file.\"}");
            if (_logger != nullptr) _logger->log("Failed to save file.\n");
            reportStep(-2);
        }
        reportStep(2);
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (file) {
            file.close();
            LittleFS.remove(upload.filename); // Clean up
        }
        server.send(500, "application/json", "{\"status\": \"nok3\", \"error\":\"File upload aborted.\"}");
        if (_logger != nullptr) _logger->log("File upload aborted.\n");
        reportStep(2);
        reportStep(-1);
    }
}

/**
 * @brief Handles directory listing via HTTP GET request.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleDirectoryList(ESP8266WebServer& server) {
    String response = "[";
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        response += "\"" + dir.fileName() + "\",";
    }
    if (response.endsWith(",")) response.remove(response.length() - 1); // Remove trailing comma
    response += "]";
    server.send(200, "application/json", response);
}


/**
 * @brief Handles file system requests via HTTP GET request.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleFileSystemRequest(ESP8266WebServer& server) {
    if (_logger != nullptr) _logger->log("handleFileSystemRequest: ");
    JsonDocument  doc;                     // Create a JSON document to store file data.
    JsonArray files = doc["files"].to<JsonArray>();

    FSInfo fs_info;
    LittleFS.info(fs_info);
    doc["total"] = fs_info.totalBytes;
    doc["used"] = fs_info.usedBytes;
    doc["free"] = fs_info.totalBytes - fs_info.usedBytes;

        // Start recursive listing from the root directory.
    listFilesRecursive("/", files);
        
        // Serialize the JSON data and send it as the response.
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    if (_logger != nullptr) _logger->log("ok\n");    
}

/**
 * @brief Handles file download requests.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleDownloadRequest(ESP8266WebServer& server) {
    String filePath = server.arg("file"); // Extract the 'file' parameter from the request if provided.
    if (_logger != nullptr) _logger->log("handleDownloadRequest '"+filePath+"'");

    if (LittleFS.exists(filePath)) {
        File file = LittleFS.open(filePath, "r"); // Attempt to open the requested file in read mode.
        server.streamFile(file, "application/octet-stream");
        file.close();
    } else {
        server.send(404, "application/json", "{\"status\": \"nok1\", \"error\":\"File not found\"}");
    }
    if (_logger != nullptr) _logger->log("\n");
   
}

/**
 * @brief Handles file deletion requests.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleDeleteRequest(ESP8266WebServer& server) {
    String path = server.arg("path");
    if (_logger != nullptr) _logger->log("handleDeleteRequest: "+path);
    if (LittleFS.exists(path)) { // Check if the file exists.
        if (LittleFS.remove(path)) { // Delete the file.
            server.send(200, "application/json", "{\"status\": \"ok\", \"message\":\"File deleted successfully\"}");
        } else {
            server.send(500, "application/json", "{\"status\": \"nok1\", \"error\":\"Failed to delete file\"}");
        }            
    } else {
        server.send(404, "application/json", "{\"status\": \"nok2\", \"error\":\"File not found\"}");
    }
    if (_logger != nullptr) _logger->log("\n");
   
}


/**
 * Handles requests to create a new directory. The request to create a direcotry 
 * is POST, with plain payload that contains an object which is JSON encoded. 
 * The object should have fields 'parentPath' and 'dirName'.
 * 
 * @brief Handles requests to create a new directory.
 * @param server Reference to the web server instance managing the request.
 */
void OTA::handleAddDirectoryRequest(ESP8266WebServer& server) {
    if (!server.hasArg("plain")) {
        server.send(400, "application/json", "{\"status\": \"nok1\", \"error\":\"Bad Request\"}");
        return;
    }
    
    String json = server.arg("plain");
    JsonDocument params;
    DeserializationError error = deserializeJson(params, json);
    if(error) {
        server.send(500, "application/json", "{\"status\": \"nok2\", \"error\":\"Failed to deserializeJson the request data\"}");
        return;
    }

    String parentPath = params["parentPath"];//server.arg("parentPath");
    String dirName =  params["dirName"];// server.arg("dirName");
    String fullPath = parentPath + "/" + dirName;
    if (_logger != nullptr) _logger->log("handleAddDirectoryRequest '"+fullPath+"': ");

    if (LittleFS.mkdir(fullPath)) {
        server.send(200, "application/json", "{\"status\": \"ok\", \"message\":\"Directory created successfully\"}");
        if (_logger != nullptr) _logger->log("ok\n");
    } else {
        server.send(500, "application/json", "{\"status\": \"nok3\", \"error\":\"Failed to create directory\"}");
        if (_logger != nullptr) _logger->log("nok\n");
    }
   
}

/**
 * Recursively retrieves all files and directories from the file system,
 * starting from the specified directory.
 *
 * @brief Recursively lists files and directories in a given path.
 * @param dirPath  The path to start listing from.
 * @param filesArray JSON array to store file and directory details.
 */
void OTA::listFilesRecursive(const String& dirPath, JsonArray& filesArray) {
    Dir dir = LittleFS.openDir(dirPath); // Open the directory at dirPath.
    while (dir.next()) {
        JsonObject file = filesArray.add<JsonObject>(); // Create a JSON object for the entry.
        file["name"] = dirPath + dir.fileName(); // Full path to the file/directory.
        file["type"] = dir.isDirectory() ? "directory" : "file"; // Type: file or directory.
        file["size"] = dir.isDirectory() ? 0 : dir.fileSize(); // Size (0 for directories).

        // If it's a directory, recurse into it.
        if (dir.isDirectory()) {
            listFilesRecursive(dirPath + dir.fileName() + "/", filesArray);
        }
    }
}

