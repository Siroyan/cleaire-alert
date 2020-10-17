#ifndef HTTPCOMMUNICATION_H_INCLUDE
#define HTTPCOMMUNICATION_H_INCLUDE

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class HttpCommunication {
    public:
        HttpCommunication();
        void setup(const char* _url, const char* _server);
        bool request();
        bool isSucceeded();
        String getRecievedData();
        DynamicJsonDocument getRecievedJson();
    private:
        uint8_t getRequestCount();
        WiFiClientSecure client;
        const char* url;
        const char* server;
        String recievedData;
        bool successFlag;
        uint16_t delayTimeMs;
        unsigned long prevRequestTime;
        uint8_t maxRequestCount;
        uint8_t requestCount;
};

#endif