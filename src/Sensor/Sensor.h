#ifndef SENSOR_H
#define SENSOR_H

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "Logger/Logger.h"
#include <vector>

#include <type_traits>


/**
 * @class Sensor
 * @brief Generic Sensor Template. The typename T is a metrics structure. Each sensor’s 
 *        metrics must implement a toJson(JsonDocument&) and optionally a isValid() methods.
 *        Sensor<T> handles all common stuff. Derived classes only need to implement 
 *        how to read sensor values.
 */
template <typename T>
class Sensor {
public:
    /**
     * @brief Constructor for Sensor.
     * @param serverManager Reference to the HTTPServerManager instance.
     * @param logger Pointer to the Logger instance.
     */
    Sensor(String sensorName, HTTPServerManager& serverManager, Logger* logger) : 
          _sensorName(sensorName),
          _serverManager(serverManager),
          _logger(logger)          
          {};

    /**
     * @brief Initializes the Sensor
     */
    virtual void begin() {
        registerEndpoints();
    }

    /**
     * @brief Main loop to read and report metrics periodically.
     */
     virtual void loop();

    /**
     * @brief Reads the current metrics from the AHT10 sensor.
     * @return The read metrics.
     */
    virtual T read() = 0; ///< child class must implement

    
    /**
     * @brief Returns the metrics that were read last time.
     * @return last metrics.
     */
    T getLastMetrics() const { return _lastMetrics; }


    /**
     * @brief Checks if the metrics are valid.
     * @param metric The metrics to check.
     * @return True if the metrics are valid, false otherwise.
     */
    virtual bool checkMetrics(const T& metric);

    /**
     * @brief Logs the metrics to the logger.
     * @param metric The metrics to log.
     */
    virtual void logMetrics(const T& metric) {
        if (_logger) {
            String json = serializeMetrics(metric);
            _logger->log("Sensor " + _sensorName + ": " + json);
        }
    }

    /**
     * @brief Broadcasts the metrics to all connected WebSocket clients.
     * @param metrics The metrics to broadcast.
     */
    void broadcastMetrics(const T& metrics){
        _serverManager.broadcastWebSocketMessage(serializeMetrics(metrics));
    };

    /**
     * @brief Adds a hook to be called during the report step.
     * @param func The function to be called.
     */
    void addReportStepHook(std::function<void(int)> func){
        _reportStepsHooks.push_back(func);
    }

    /**
     * @brief Reports the current step to all registered hooks.
     * @param step The step to report.
     */
    void reportStep(int step);


    /**
     * @brief Adds a hook to be called when metrics are reported.
     * @param func The function to be called.
     */
    void addReportMetricsHook(std::function<void(T&)> func){
        _reportMetricsHooks.push_back(func);
    };

    /**
     * @brief Reports the metrics to all registered hooks.
     * @param metric The metrics to report.
     */
    void reportMetrics(T& metric);

    /**
     * @brief Registers endpoints for the HTTP server.
     */
    void registerEndpoints();
    

    void setPeriodicity(int periodicity){_periodicity = periodicity;}


protected:
    String _sensorName = "GenericSensor"; ///< Reference name of the metrics, used in registring HTTP end point 
    HTTPServerManager& _serverManager; ///< Reference to the HTTPServerManager instance.
    Logger* _logger; ///< Pointer to the Logger instance.
    
    long _lastTime = 0; ///< Last time metrics were read.
    int _periodicity = 10000; ///< Periodicity of reading metrics in milliseconds.
    
    T _lastMetrics; ///< Last read metrics.

    std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of report step hooks.
    std::vector<std::function<void(T&)>> _reportMetricsHooks; ///< List of report metrics hooks.

    /**
     * @brief Serializes the metrics to a JSON string.
     * @param metrics The metrics to serialize.
     * @return The serialized JSON string.
     */
    String serializeMetrics(const T& metrics);

};





template <typename T>
void Sensor<T>::loop() {
    long currentTime = millis();
    if (currentTime - _lastTime > _periodicity) {
        _lastTime = currentTime;
        reportStep(1); 
        _lastMetrics = read();
        logMetrics(_lastMetrics);
        reportMetrics(_lastMetrics);
        broadcastMetrics(_lastMetrics);
        reportStep(2);        
    }   
}

// Helper trait to detect if T has isValid() method
template <typename, typename = void>
struct has_isValid : std::false_type {};

template <typename T>
struct has_isValid<T, std::void_t<decltype(std::declval<T>().isValid())>> : std::true_type {};

/**
 * Using SFINAE / C++17 constexpr to call method isValid() if T has it. If not, 
 * return true.
*/
template <typename T>
bool Sensor<T>::checkMetrics(const T& metrics) {
    if constexpr (has_isValid<T>::value) {
        // If isValid() exists
        return metrics.isValid();
    } else {
        // Fallback: always true
        return true;
    }
}


template <typename T>
void Sensor<T>::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}

template <typename T>
void Sensor<T>::reportMetrics(T& metric){
  for (auto& hook : _reportMetricsHooks) {
    if (hook) {
        hook(metric);
    }
  }
}

template <typename T>
void Sensor<T>::registerEndpoints() {
    _serverManager.registerPage("/api/getLastMetrics"+_sensorName, HTTP_GET, [this](ESP8266WebServer& server) {
        server.send(200, "application/json", serializeMetrics(_lastMetrics));
    });

    _serverManager.registerPage("/api/getPeriodicity"+_sensorName, HTTP_GET, [this](ESP8266WebServer& server) {
        server.send(200, "application/json", "{\"periodicity\":\""+String(_periodicity)+"\"}");
    });

    _serverManager.registerPage("/api/setPeriodicity"+_sensorName, HTTP_POST, [this](ESP8266WebServer& server) {
        if (!server.hasArg("plain")) {
            server.send(400, "application/json", "{\"status\": -1, \"error\":\"Bad Request\"}");
            return;
        }
        
        String json = server.arg("plain");
        JsonDocument params;
        DeserializationError error = deserializeJson(params, json);
        if(error) {
            server.send(500, "application/json", "{\"status\": -2, \"error\":\"Failed to deserializeJson the request data\"}");
            return;
        }

        _periodicity = params["periodicity"];
        if (_logger != nullptr) _logger->logf("/api/setPeriodicity%s %d\n", _sensorName.c_str(), _periodicity);
        server.send(200, "application/json", "{\"status\": 1, \"message\":\"Periodicity for the sensor "+_sensorName+" is set\"}");
   
    });

}


template <typename T>
String Sensor<T>::serializeMetrics(const T& metrics) {
    JsonDocument doc;
    doc["sensor"] = _sensorName;
    metrics.toJson(doc);   // delegate to the struct
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}
#endif