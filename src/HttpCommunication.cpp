#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"

HttpCommunication::HttpCommunication() {
    requestCount = 0;
    prevRequestTime = millis();
}

void HttpCommunication::setup(const char* _url, const char* _server) {
    url = _url;
    server = _server;
    maxRequestCount = 20;
}

void HttpCommunication::setup(const char* _url, const char* _server, uint8_t _maxRequestCount) {
    url = _url;
    server = _server;
    maxRequestCount = _maxRequestCount;
}

bool HttpCommunication::request() {
    successFlag = false;

    if ((millis() - prevRequestTime) < 1000) return false;
    prevRequestTime = millis();

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

DynamicJsonDocument HttpCommunication::getRecievedJson() {
    String receivedText = HttpCommunication::getRecievedData();
	// Clean up http responce. Get JSON only.
	receivedText.remove(0, 3);
	receivedText.trim();
	receivedText.remove(receivedText.length()-1, 1);
	// convert to json
	const size_t capacity = 3000;
	DynamicJsonDocument doc(capacity);
	DeserializationError error = deserializeJson(doc, receivedText);
	if (error) Serial.println("deserializeJson() failed: " + String(error.c_str()));
	return doc;
}