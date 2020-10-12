#ifndef LIVESTARTTIME_H_INCLUDE
#define LIVESTARTTIME_H_INCLUDE

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"

class LiveStartTime : public HttpCommunication {
    public:
        const char* getStartTime();
};

#endif