#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"
#include "UpcomingLive.h"

const char* UpcomingLive::getThumbnailUrl() {
	DynamicJsonDocument searchResultJson = HttpCommunication::getRecievedJson();
	return searchResultJson["items"][0]["snippet"]["thumbnails"]["medium"]["url"];
}

const char* UpcomingLive::getLiveId() {
	DynamicJsonDocument searchResultJson = HttpCommunication::getRecievedJson();
	return searchResultJson["items"][0]["id"]["videoId"];
}

bool UpcomingLive::isExist() {
	DynamicJsonDocument searchResultJson = HttpCommunication::getRecievedJson();
	
	if (searchResultJson["items"] != NULL) return false;
	else return true;
}
