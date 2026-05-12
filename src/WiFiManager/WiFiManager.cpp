
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
    //_logger->logError("[WifiManager] Operation mode: SETTINGS.");
    _lastTime = currentTime;
    if(reConnecToAP()){
      reboot();
    }
  }
  if(_operationMode==NORMAL && !reConnecToAP()) { // switch mode
    _operationMode=SETTINGS;
    _logger->logError("[WifiManager] Changed operation mode from normal to settings.");
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
    if (_logger != nullptr) _logger->logError("[WifiManager] WiFi is disconnected.");
    
    WiFi.disconnect();  // Ensure a clean start for reconnection
    return this->connectToAP();
  }
  return true;
}

bool WiFiManager::connectToAP() {
    if (strlen(_SSID) == 0 || strlen(_password) == 0) {
        if (_logger != nullptr) _logger->logError("[WifiManager] WiFi credentials are missing.");
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
            //if (_logger != nullptr) _logger->logError("[WifiManager] Failed to connect to WiFi.");
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

    int RSSI = WiFi.RSSI(); //< Received Signal Strength Indicator in dBm (decibel-milliwatts)
    if (_logger != nullptr) _logger->logSystem("Connected to WiFi! IP Address: %s. Signal strength %d dBm which is %s.", WiFi.localIP().toString().c_str(), RSSI, interpretSignalStrength(RSSI).c_str());

    return true;
}

void WiFiManager::createAP() {
  if(!_APstarted){
    if (_logger != nullptr) _logger->logSystem("Creating AP with SSID: %s", _apSSID);


    _APstarted = WiFi.softAP(_apSSID, _apPassword);
    
    if (_logger != nullptr) {
      _logger->logSystem("AP IP Address: : %s", WiFi.softAPIP().toString().c_str());
    }
  }
}

void WiFiManager::reboot() {
    // Reboot the ESP8266
    if (_logger != nullptr) _logger->logError("[WifiManager] Rebooting...");
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
    _serverManager.registerPage("/api/nearby-ap/detailed", HTTP_GET, [this](ESP8266WebServer& server) { handleScanAPs(server, true); });
}

void WiFiManager::handleScanAPs(ESP8266WebServer& server, bool detailed) {
    int n = WiFi.scanNetworks();  // Perform Wi-Fi scan
    String response = "[";
    for (int i = 0; i < n; ++i) {
        String SSID = WiFi.SSID(i);
        int RSSI = WiFi.RSSI(i); //< Received Signal Strength Indicator in dBm (decibel-milliwatts)
        int channel = WiFi.channel(i);
        String BSSID = WiFi.BSSIDstr(i);
        uint8_t encryptionType = WiFi.encryptionType(i);
        bool hidden = WiFi.isHidden(i);
        if(detailed) {
          response += "{\"SSID\": \"" + SSID + "\", \"BSSID\": \"" + BSSID + "\", \"RSSI\": " + RSSI + ", \"SiganalStrength\": \"" + interpretSignalStrength(RSSI) + "\", \"channel\": " + channel + ", \"encryptionType\": \"" + getEncryptionTypeString(encryptionType) + "\", \"hidden\": " + ((hidden)?"true":"false") + "}";
        }
        else {
          response += "\"" + SSID + " ["+RSSI+" dBm, wich is "+interpretSignalStrength(RSSI)+"] (channel: "+String(channel)+")\"";
        }
        if (i < n - 1) response += ",";        
    }
    response += "]";
    server.send(200, "application/json", response);
}

String WiFiManager::interpretSignalStrength(int dBm) {
    if (dBm >= -50) {
        return "Excellent";
    }
    else if (dBm >= -60) {
        return "Great";
    }
    else if (dBm >= -67) {
        return "Good";
    }
    else if (dBm >= -70) {
        return "Fair";
    }
    else if (dBm >= -80) {
        return "Poor";
    }
    else if (dBm >= -90) {
        return "Very Poor";
    }
    else {
        return "No Signal";
    }
}

String WiFiManager::getEncryptionTypeString(uint8_t encryptionType) {
  switch (encryptionType) {
    case ENC_TYPE_NONE:
      return "Open";
    case ENC_TYPE_TKIP:
      return "WPA/TKIP";
    case ENC_TYPE_CCMP:
      return "WPA2/AES (CCMP)";
    case ENC_TYPE_AUTO:
      return "Auto (WPA/WPA2 Mixed)";
    case ENC_TYPE_WEP:
      return "WEP";
    #ifdef ENC_TYPE_TKIP_WPA_MIXED  // Some ESP8266 versions include this
    case ENC_TYPE_TKIP_WPA_MIXED:
      return "WPA/TKIP Mixed";
    #endif
    default:
      return "Unknown Encryption Type";
  }
}