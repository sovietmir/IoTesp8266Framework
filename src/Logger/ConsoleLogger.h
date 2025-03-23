#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include "Logger.h"

class ConsoleLogger : public Logger {
public:
    void log(const char* message) override;
};

#endif