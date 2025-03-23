/**
 * @file MqttManager.cpp
 * @brief Implementation of the MqttManager class for MQTT communication handling.
 */
#include "MqttManager/MqttManager.h"

/**
 * @brief Construct a new MqttManager object.
 * @param logger Pointer to the Logger instance.
 */
MqttManager::MqttManager(Logger* logger):  
    _logger(logger),
    _espClient(), 
    _client(_espClient)
    {}

/**
 * @brief Initializes MQTT client settings and connects to the broker.
 */
void MqttManager::begin(){
    _client.setServer(_server, _port);      
}

/**
 * @brief Processes MQTT loop and manages reconnection.
 */
void  MqttManager::loop(){ 
    if (!_client.loop()) {
      _client.connect(_clientId, _username, _password);
    }
}

/**
 * @brief Publishes a message to the MQTT broker.
 * @return bool True if successful, false otherwise.
 */
bool MqttManager::publish(String topic, String value) {
  if (_client.connected()){
    _client.publish((_topicPrefix +"/"+ topic).c_str(), value.c_str());
    return true;
  }
  reportStep(-1);
  return false;
} 

/**
 * @brief Adds a callback function to the reporting hooks list.
 * @param func Callback function to add.
 */
void MqttManager::addReportStepHook(std::function<void(int)> func) {
  _reportStepsHooks.push_back(func);
}

/**
 * @brief Executes all registered reporting hooks with the given step value.
 * @param step Step value to report.
 */
void MqttManager::reportStep(int step){
  for (auto& hook : _reportStepsHooks) {
    if (hook) {
        hook(step);
    }
  }
}