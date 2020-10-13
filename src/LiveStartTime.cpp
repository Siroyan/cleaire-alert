#include <Arduino.h>
#include <ArduinoJson.h>
#include "HttpCommunication.h"
#include "LiveStartTime.h"

const char* LiveStartTime::getStartTime() {
    // 配信開始時間を取得
    DynamicJsonDocument upComingLiveDetailJson = HttpCommunication::getRecievedJson();
    return upComingLiveDetailJson["items"][0]["liveStreamingDetails"]["scheduledStartTime"];
}