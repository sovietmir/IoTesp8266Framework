
#include "WiFiManager/WiFiManager.h"

WiFiManager::WiFiManager(HTTPServerManager& serverManager, Logger* logger) 
: _serverManager(serverManager), 
  _logger(logger),
  _operationMode(INIT)
  {}

void WiFiManager::begin() {    
  // Configure Network Time Protocol
  //  ex. _timeZone = "EET-2EEST,M3.5.0/3,M10.5.0/4" which is TZ_Europe_Athens
  //  ex. _timeServer = "pool.ntp.org"
  // configTime() is core ESP8266-specific function
  configTime(_timeZone, _timeServer); 

  if(!firstConnectToAP()) { // switch mode 
    _operationMode=SETTINGS;;   
  }
  else { // Connected to WIFI, do DateTime 
    _operationMode=NORMAL;
    //if (_logger != nullptr) _logger->logf("Date Now is %s\n", Logger::timeToString().c_str());
  }

  registerEndpoints();
}

void WiFiManager::loop(){
  unsigned long currentTime = millis();
  if(_operationMode==SETTINGS && currentTime - _lastTime > 60000) {
    _lastTime = currentTime;
    if(reConnecToAP()){
      reboot();
    }
  }
  if(_operationMode==NORMAL && !reConnecToAP()) { // switch mode
    _operationMode=SETTINGS;
    reportStep(-4);// errorLED.setMode(BLINK); 
  }
  if(_operationMode==SETTINGS){
    createAP();
  }
}

bool WiFiManager::firstConnectToAP() {
  if (strlen(_hostname) > 0) {
    WiFi.hostname(_hostname);
  }
  return this->connectToAP();
}

bool WiFiManager::reConnecToAP() {
  if (WiFi.status() != WL_CONNECTED) {  // Check if Wi-Fi is disconnected
    if (_logger != nullptr) _logger->log("WiFi disconnected.\n");
    
    WiFi.disconnect();  // Ensure a clean start for reconnection
    return this->connectToAP();
  }
  return true;
}

bool WiFiManager::connectToAP() {
    if (strlen(_SSID) == 0 || strlen(_password) == 0) {
        if (_logger != nullptr) _logger->log("WiFi credentials are missing.\n");
        reportStep(-1);
        return false;
    }

    if (_logger != nullptr) _logger->log("Connecting to WiFi.\n");
    if (_logger != nullptr) _logger->logf("SSID: %s\n", _SSID);

    reportStep(1);
    WiFi.begin(_SSID, _password);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        reportStep(2);
        if (millis() - startTime > 10000) {            
            if (_logger != nullptr) _logger->log("Failed to connect to WiFi.\n");
            reportStep(-2);
            return false;
        }
        delay(200);
        reportStep(3);
        delay(100);
        if (_logger != nullptr) _logger->log(".");        
    }
    if (_logger != nullptr) _logger->log("\n");
    reportStep(4); 

    if (_logger != nullptr) _logger->logf("Connected to WiFi! IP Address: %s\n", WiFi.localIP().toString().c_str());
    return true;
}

void WiFiManager::createAP() {
  if(!_APstarted){
    if (_logger != nullptr) _logger->logf("Creating AP with SSID: %s\n", _apSSID);

    _APstarted = WiFi.softAP(_apSSID, _apPassword);
    
    if (_logger != nullptr) _logger->logf("AP IP Address: : %s\n", WiFi.softAPIP().toString().c_str());
  }
}

void WiFiManager::reboot() {
    // Reboot the ESP8266
    if (_logger != nullptr) _logger->log("Rebooting...\n");
    ESP.restart();  // This will reset the ESP8266
}

void WiFiManager::addReportStepHook(std::function<void(int)> func) {
  _reportStepsHooks.push_back(func);
}

void WiFiManager::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}

void WiFiManager::registerEndpoints() {
    _serverManager.registerPage("/api/nearby-ap", HTTP_GET, [this](ESP8266WebServer& server) { handleScanAPs(server); });
}

void WiFiManager::handleScanAPs(ESP8266WebServer& server) {
    int n = WiFi.scanNetworks();  // Perform Wi-Fi scan
    String response = "[";
    for (int i = 0; i < n; ++i) {
        response += "\"" + WiFi.SSID(i) + "\"";
        if (i < n - 1) response += ",";
    }
    response += "]";
    server.send(200, "application/json", response);
}