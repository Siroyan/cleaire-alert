#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"
#include "UpcomingLive.h"

DynamicJsonDocument convertToJson(String receivedText);

DynamicJsonDocument UpcomingLive::getRecievedJson() {
    return convertToJson(HttpCommunication::getRecievedData());
}

const char* UpcomingLive::getThumbnailUrl() {
	DynamicJsonDocument searchResultJson = getRecievedJson();
	return searchResultJson["items"][0]["snippet"]["thumbnails"]["medium"]["url"];
}

const char* UpcomingLive::getLiveId() {
	DynamicJsonDocument searchResultJson = getRecievedJson();
	return searchResultJson["items"][0]["id"]["videoId"];
}

bool UpcomingLive::isExist() {
	DynamicJsonDocument searchResultJson = getRecievedJson();
	
	if (searchResultJson["items"] != NULL) return false;
	else return true;
}

DynamicJsonDocument convertToJson(String receivedText) {
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