#ifndef UPCOMINGLIVE_H_INCLUDE
#define UPCOMINGLIVE_H_INCLUDE

#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"

class UpcomingLive : public HttpCommunication {
    public:
        DynamicJsonDocument getRecievedJson();
        bool isExist();
        const char* getThumbnailUrl();
        const char* getLiveId();
};

#endif