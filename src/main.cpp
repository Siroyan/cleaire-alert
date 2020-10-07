#include <Arduino.h>
#include <M5Stack.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include <Credentials.h>
#include <HttpCommunication.h>

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

#define M_IMG_WIDTH  (320)
#define M_IMG_HEIGHT (180)

bool httpCommunication(char* url, char* server);
DynamicJsonDocument convertToJson(String receivedText);
void displayThumbnail(DynamicJsonDocument searchResultJson);
void displayTimer(DynamicJsonDocument searchResultJson);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PW;

const char* apiKey = API_KEY;
const char* ytSearchApiUrlNoKey = "https://www.googleapis.com/youtube/v3/search?part=snippet&channelId=UC1zFJrfEKvCixhsjNSb1toQ&eventType=upcoming&maxResults=1&type=video&order=date&key=";
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
  lcd.clear();
  
  // Serial comm to PC
  Serial.begin(115200);
  while(!Serial);
  delay(1000);

  // Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Connected to " + String(ssid));
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
        displayTimer(searchResultJson);
        // カウントダウンモードへ移行
        if (YtThumbnail.isSucceeded() && YtVideoApi.isSucceeded()) mode = 1;
      }
    }
    delay(10000);
  } else if (mode == 1){
    // 配信までのカウントダウン
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

void displayTimer(DynamicJsonDocument searchResultJson) {
  // 配信予定の詳細を取得
  const char* id = searchResultJson["items"][0]["id"]["videoId"];
  String ytVideoApiUrl = String(ytVideoApiUrlNoIdNoKey) + String(id) + "&key=" +  String(apiKey);

  YtVideoApi.setup((char*)ytVideoApiUrl.c_str(), (char*)ytServer);
  if (YtVideoApi.request()) {
    String recievedLiveDetail = YtVideoApi.getRecievedData();
    DynamicJsonDocument upComingLiveDetailJson = convertToJson(recievedLiveDetail);
    scheduledStartTime = upComingLiveDetailJson["items"][0]["liveStreamingDetails"]["scheduledStartTime"];
  }

  lcd.setTextSize(2.5);
  lcd.drawString("The live starts at", 10, M_IMG_HEIGHT + 10); 
  lcd.drawString(scheduledStartTime, 10, M_IMG_HEIGHT + 35);
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