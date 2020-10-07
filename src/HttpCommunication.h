#ifndef HTTPCOMMUNICATION_H_INCLUDE
#define HTTPCOMMUNICATION_H_INCLUDE

#include <Arduino.h>
#include <WiFiClientSecure.h>

class HttpCommunication {
    public:
        HttpCommunication();
        void setup(const char* _url, const char* _server);
        void setup(const char* _url, const char* _server, uint8_t _maxRequestCount);
        bool request();
        uint8_t getRequestCount();
        bool isSucceeded();
        String getRecievedData();
    private:
        WiFiClientSecure client;
        const char* url;
        const char* server;
        String recievedData;
        bool successFlag;
        uint8_t maxRequestCount;
        uint8_t requestCount;
};

#endif