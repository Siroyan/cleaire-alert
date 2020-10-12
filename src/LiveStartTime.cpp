#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"
#include "LiveStartTime.h"

const char* LiveStartTime::getStartTime() {
    // 配信開始時間を取得
    String recievedLiveDetail = HttpCommunication::getRecievedData();
    DynamicJsonDocument upComingLiveDetailJson = convertToJson(recievedLiveDetail);
    return upComingLiveDetailJson["items"][0]["liveStreamingDetails"]["scheduledStartTime"];
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