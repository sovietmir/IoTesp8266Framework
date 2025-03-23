/**
 * @file MqttManager.h
 * @brief Manages MQTT client connections, message publishing, and callback hooks.
 * 
 * Handles interactions with an MQTT broker, including connection setup, topic management,
 * and user-defined reporting hooks.
 */
#ifndef WQTT_MANAGER_H
#define WQTT_MANAGER_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Stream.h>
#include <vector>
//#include "common.h"
#include "Logger/Logger.h"


class MqttManager {
public:
    /**
     * @brief Construct a new MqttManager object.
     * @param logger Pointer to the Logger instance for logging events.
     */
    MqttManager(Logger* logger);
   
    /**
     * @brief Initializes the MQTT client with the configured server and port.
     */
    void begin();
    
    /**
     * @brief Maintains MQTT connection and processes incoming messages.
     * @details Automatically reconnects to the broker if disconnected.
     */
    void loop();

    /**
     * @brief Sets the base topic for MQTT messages.
     * @param value The base topic string (e.g., "EnergyMonitor").
     */
    void setTopicPrefix(const char* IoTclassName, const char* IoTName){ _topicPrefix = String(IoTclassName)+"/"+String(IoTName);};
    
    /**
     * @brief Sets the MQTT broker server address.
     * @param value Broker IP or hostname (e.g., "mqtt.eclipse.org").
     */
    void setServer(const char* value){ _server = value;};
    
    /**
     * @brief Sets the MQTT broker port.
     * @param value Port number (e.g., 1883).
     */
    void setPort(int value){ _port = value;}
    
    /**
     * @brief Sets the MQTT client ID.
     * @param value Unique client identifier.
     */
    void setClientId(const char* value){ _clientId = value;}      
    
    /**
     * @brief Sets the MQTT username for authentication.
     * @param value Username string.
     */
    void setUsername(const char* value){ _username = value;}
    
    /**
     * @brief Sets the MQTT password for authentication.
     * @param value Password string.
     */
    void setPassword(const char* value){ _password = value;};

    /**
     * @brief Publishes a message to the MQTT broker.
     * @param topic Subtopic under the base topic (e.g., "status").
     * @param value Message payload to publish.
     * @return true Message sent successfully.
     * @return false Client not connected to the broker.
     */
    bool publish(String topic, String value);
    
    /**
     * @brief Registers a callback function to report execution steps.
     * @param func Callback function accepting an integer step parameter.
     */
    void addReportStepHook(std::function<void(int)> func);
    
    /**
     * @brief Triggers all registered step-reporting callbacks.
     * @param step Step value to report (e.g., -1 for errors, 1/2 for progress).
     */
    void reportStep(int step);

private:
    Logger* _logger; ///< Logger instance for event logging.
    WiFiClient _espClient; ///< Underlying WiFi client for MQTT.
    PubSubClient _client; ///< MQTT client instance.
    String _topicPrefix = "IoT"; ///< Base topic prefix for messages.
    const char *_topic, *_server, *_clientId, *_username, *_password; ///< MQTT configuration parameters.
    int _port; ///< MQTT broker port number.
    std::vector<std::function<void(int)>> _reportStepsHooks; ///< List of registered step-reporting hooks.
};

#endif