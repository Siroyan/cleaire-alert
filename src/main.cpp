#include <Arduino.h>
#include <M5Stack.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include <Credentials.h>
#include <HttpCommunication.h>
#include "time.h"

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

#define M_IMG_WIDTH  (320)
#define M_IMG_HEIGHT (180)

bool httpCommunication(char* url, char* server);
DynamicJsonDocument convertToJson(String receivedText);
void displayThumbnail(DynamicJsonDocument searchResultJson);
void getLiveStartTime(DynamicJsonDocument searchResultJson);
void displayTimer();
long convertToUnixTime(const char* iso8601Time);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PW;

const char* ntpServer = "ntp.nict.jp";

const char* apiKey = API_KEY;
const char* ytSearchApiUrlNoKey = "https://www.googleapis.com/youtube/v3/search?part=snippet&channelId=UCjlmCrq4TP1I4xguOtJ-31w&eventType=upcoming&maxResults=1&type=video&order=date&key=";
const char* ytVideoApiUrlNoIdNoKey = "https://www.googleapis.com/youtube/v3/videos?part=liveStreamingDetails&id=";
const char* ytServer = "www.googleapis.com";
const char* imageServer = "i.ytimg.com";
const char* scheduledStartTime;

uint8_t mode = 0;
HttpCommunication YtSearchApi;
HttpCommunication YtVideoApi;
HttpCommunication YtThumbnail;

void setup() {
  // LCD Setting
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(24);
  lcd.clear();
  lcd.setFont(&fonts::lgfxJapanGothic_36);
  
  // Serial comm to PC
  Serial.begin(115200);
  delay(1000);

  // Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("Connected to " + String(ssid));

  configTime(0, 0, ntpServer);
}

void loop() {
  if (mode == 0) {
    String ytSearchApiUrl = String(ytSearchApiUrlNoKey) + String(apiKey);
    YtSearchApi.setup((char*)ytSearchApiUrl.c_str(), (char*)ytServer);
    if (YtSearchApi.request()) {
      String recievedSearchResult = YtSearchApi.getRecievedData();
      DynamicJsonDocument searchResultJson = convertToJson(recievedSearchResult);

      // 配信予定があるか
      if (searchResultJson["items"] != NULL) {
        displayThumbnail(searchResultJson);
        getLiveStartTime(searchResultJson);
        // カウントダウンモードへ移行
        if (YtThumbnail.isSucceeded() && YtVideoApi.isSucceeded()) mode = 1;
      }
    }
  } else if (mode == 1){
    // 配信までのカウントダウン
    displayTimer();
  }
}

void displayThumbnail(DynamicJsonDocument searchResultJson) {
  const char* thumbnailUrl = searchResultJson["items"][0]["snippet"]["thumbnails"]["medium"]["url"];
  // サムネイルのデータを取得
  YtThumbnail.setup((char*)thumbnailUrl, (char*)imageServer);
  if (YtThumbnail.request()) {
    String recievedImgString = YtThumbnail.getRecievedData();
    uint16_t jpgDataLength = recievedImgString.length();

    // サムネ画像のバイナリデータを作成
    uint8_t* jpgData = new uint8_t[jpgDataLength];
    for (int i=0; i < jpgDataLength; i++) jpgData[i] = recievedImgString.charAt(i);

    // サムネを表示
    lcd.drawJpg(jpgData, jpgDataLength, 0, 0, M_IMG_WIDTH, M_IMG_HEIGHT);
    free(jpgData);
  }
}

void getLiveStartTime(DynamicJsonDocument searchResultJson) {
  // 配信予定の詳細を取得
  const char* id = searchResultJson["items"][0]["id"]["videoId"];
  String ytVideoApiUrl = String(ytVideoApiUrlNoIdNoKey) + String(id) + "&key=" +  String(apiKey);

  YtVideoApi.setup((char*)ytVideoApiUrl.c_str(), (char*)ytServer);
  if (YtVideoApi.request()) {
    // 配信開始時間を取得
    String recievedLiveDetail = YtVideoApi.getRecievedData();
    DynamicJsonDocument upComingLiveDetailJson = convertToJson(recievedLiveDetail);
    scheduledStartTime = upComingLiveDetailJson["items"][0]["liveStreamingDetails"]["scheduledStartTime"];
  }
}

void displayTimer() {
  // 現在時刻から配信開始までの時間を計算
long diffTime = convertToUnixTime(scheduledStartTime) - time(NULL);
  if (diffTime > 24 * 60 * 60) {
    // 一日以上後
    lcd.drawString("一日以上先の配信です", 10, M_IMG_HEIGHT + 10);
  } else if (diffTime <= 0) { 
    lcd.drawString("配信中!", 100, M_IMG_HEIGHT + 10);
  } else {
    uint8_t diffH = diffTime / 3600;
    uint8_t diffM = (diffTime % 3600) / 60;
    uint8_t diffS = diffTime % 60;
    String h = diffH < 10 ? "0" + String(diffH) : String(diffH);
    String m = diffM < 10 ? "0" + String(diffM) : String(diffM);
    String s = diffS < 10 ? "0" + String(diffS) : String(diffS);
    lcd.drawString("開始まで " + h + ":" + m + ":" + s, 5, M_IMG_HEIGHT + 10);
  }
}

long convertToUnixTime(const char* iso8601Time) {
  uint16_t totalDays[12] = {0,31,59,90,120,151,181,212,243,273,304,334};

  uint16_t y  = String(iso8601Time).substring( 0,  4).toInt();
  uint8_t  mo = String(iso8601Time).substring( 5,  7).toInt();
  uint8_t  d  = String(iso8601Time).substring( 8, 10).toInt();
  uint8_t  h  = String(iso8601Time).substring(11, 13).toInt();
  uint8_t  mi = String(iso8601Time).substring(14, 16).toInt();
  uint8_t  s  = String(iso8601Time).substring(17, 19).toInt();

  long unixTime = 0;
  for (uint16_t year = 1970; year < y; year++) {
    if ((year % 400) == 0 || ((year % 4) == 0 && (year % 100) != 0)) unixTime += 366 * 24 * 60 * 60;
    else unixTime += 365 * 24 * 60 * 60;
  }
  unixTime += totalDays[mo-1] * 24 * 60 * 60;
  unixTime += d * 24 * 60 * 60;
  unixTime += h * 60 * 60;
  unixTime += mi * 60;
  unixTime += s;

  return unixTime;
}

DynamicJsonDocument convertToJson(String receivedText) {
  // Clean up http responce. Get JSON only.
  receivedText.remove(0, 3);
  receivedText.trim();
  receivedText.remove(receivedText.length()-1, 1);
  // convert to json
  const size_t capacity = 2000;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, receivedText);
  if (error) Serial.println("deserializeJson() failed: " + String(error.c_str()));
  return doc;
}