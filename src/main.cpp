#include <Arduino.h>
#include <M5Stack.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include <Credentials.h>

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

#define M_IMG_WIDTH  (320)
#define M_IMG_HEIGHT (180)

bool httpCommunication(char* url, char* server);
DynamicJsonDocument convertToJson(String receivedText);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PW;

const char* apiKey = API_KEY;
const char* ytSearchApiUrlNoKey = "https://www.googleapis.com/youtube/v3/search?part=snippet&channelId=UC1zFJrfEKvCixhsjNSb1toQ&eventType=upcoming&maxResults=1&type=video&order=date&key=";
const char* ytVideoApiUrlNoIdNoKey = "https://www.googleapis.com/youtube/v3/videos?part=liveStreamingDetails&id=";
const char* ytServer = "www.googleapis.com";
const char* imageServer = "i.ytimg.com";
const char* scheduledStartTime;

uint8_t mode = 0;

WiFiClientSecure client;

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
    
    // Get movie thumbnail url from youtube search api and get start time of upcoming-live from video api
    String ytSearchApiUrl = String(ytSearchApiUrlNoKey) + String(apiKey);
    Serial.println(ytSearchApiUrl);
    
    bool isSucceededInYtSearchApiConn = httpCommunication((char*)ytSearchApiUrl.c_str(), (char*)ytServer);
      if (isSucceededInYtSearchApiConn) {
      String recievedSearchResult = client.readString();
      DynamicJsonDocument searchResultJson = convertToJson(recievedSearchResult);
      // 配信予定があるか
      if (searchResultJson["items"] != NULL) {
        const char* thumbnailUrl = searchResultJson["items"][0]["snippet"]["thumbnails"]["medium"]["url"];
        Serial.println(String(thumbnailUrl));
        // サムネイルのデータを取得
        client.stop();
        bool isSucceededInImgConn = httpCommunication((char*)thumbnailUrl, (char*)imageServer);
        if (isSucceededInImgConn) {
          String recievedImgString = client.readString();
          uint16_t jpgDataLength = recievedImgString.length();

          // サムネ画像のバイナリデータを作成
          uint8_t* jpgData = new uint8_t[jpgDataLength];
          for (int i=0; i < jpgDataLength; i++) jpgData[i] = recievedImgString.charAt(i);

          // サムネを表示
          lcd.drawJpg(jpgData, jpgDataLength, 0, 0, M_IMG_WIDTH, M_IMG_HEIGHT);
          free(jpgData);
        }

        // 配信予定の詳細を取得
        const char* id = searchResultJson["items"][0]["id"]["videoId"];
        
        String ytVideoApiUrl = String(ytVideoApiUrlNoIdNoKey) + String(id) + "&key=" +  String(apiKey);
        Serial.println(ytVideoApiUrl);
        client.stop();
        bool isSucceededYtVideoApiConn = httpCommunication((char*)ytVideoApiUrl.c_str(), (char*)ytServer);
        if (isSucceededYtVideoApiConn) {
          String recievedLiveDetail = client.readString();
          DynamicJsonDocument upComingLiveDetailJson = convertToJson(recievedLiveDetail);
          scheduledStartTime = upComingLiveDetailJson["items"][0]["liveStreamingDetails"]["scheduledStartTime"];
        }

        lcd.setTextSize(2.5);
        lcd.drawString("Live starts at...", 10, M_IMG_HEIGHT + 10); 
        lcd.drawString(scheduledStartTime, 10, M_IMG_HEIGHT + 35);

        // カウントダウンモードへ移行
        if (isSucceededInImgConn && isSucceededYtVideoApiConn) mode = 1;
      }
    }
    client.stop();
    delay(10000);
  } else if (mode == 1){
    // 配信までのカウントダウン
  }
}

bool httpCommunication(char* url, char* server) {
  Serial.println("Try to connect " + String(server) + "...");
  if (!client.connect(server, 443)) {
    Serial.println("Connection failed!");
    return false;
  }
  Serial.println("Connected to server!");

  // Make a HTTP request:
  client.println("GET " + String(url) + " HTTP/1.1");
  client.println("Host: " + String(server));
  client.println("Connection: close");
  client.println();

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.println("Unexpected response: " + String(status));
    return false;
  }
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
    return false;
  }
  return true;
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