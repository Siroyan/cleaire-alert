#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "HttpCommunication.h"

HttpCommunication::HttpCommunication() {
    // hoge
}

void HttpCommunication::setup(const char* _url, const char* _server) {
    url = _url;
    server = _server;
    delayTimeMs = 1000;
    maxRequestCount = 10;
}

void HttpCommunication::setup(const char* _url, const char* _server, uint8_t _maxRequestCount) {
    url = _url;
    server = _server;
    delayTimeMs = 1000;
    maxRequestCount = _maxRequestCount;
}

bool HttpCommunication::request() {
    successFlag = false;
    requestCount++;
    // Googleに怒られないように
    if (millis() - prevRequestTime < delayTimeMs) return false;
    if (getRequestCount() > maxRequestCount) return false;

    if (!client.connect(server, 443)) return false;
    
    // Send the HTTP request:
    client.println("GET " + String(url) + " HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Connection: close");
    client.println();

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) return false;
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) return false;
    
    recievedData = client.readString();
    client.stop();

    successFlag = true;
    requestCount = 0;
    prevRequestTime = millis();
    return true;
}

uint8_t HttpCommunication::getRequestCount() {
    return requestCount;
}

bool HttpCommunication::isSucceeded() {
    return successFlag;
}

String HttpCommunication::getRecievedData() {
    return recievedData;
}