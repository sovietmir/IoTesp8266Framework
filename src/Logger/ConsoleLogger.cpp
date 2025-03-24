#include "ConsoleLogger.h"
#include <Arduino.h>

void ConsoleLogger::begin() {
    Serial.begin(115200);
}

void ConsoleLogger::log(const char* message) {
    Serial.print(message);
}
