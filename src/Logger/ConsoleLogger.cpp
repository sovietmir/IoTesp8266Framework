#include "ConsoleLogger.h"
#include <Arduino.h>

void ConsoleLogger::log(const char* message) {
    Serial.print(message);
}
