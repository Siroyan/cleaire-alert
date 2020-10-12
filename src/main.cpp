#include <Arduino.h>
#include <M5Stack.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include "Credentials.h"
#include "HttpCommunication.h"
#include "UpcomingLive.h"
#include "Thumbnail.h"
#include "LiveStartTime.h"
#include "time.h"

#define M_IMG_WIDTH 320
#define M_IMG_HEIGHT 180

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

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
UpcomingLive upcomingLive;
Thumbnail thumbnail;
LiveStartTime liveStartTime;

void setup() {
	// LCD Setting
	lcd.init();
	lcd.setRotation(1);
	lcd.setBrightness(128);
	lcd.setColorDepth(24);
	lcd.clear();
	lcd.setFont(&fonts::lgfxJapanGothic_36);

	// PCと
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
		upcomingLive.setup((char*)ytSearchApiUrl.c_str(), (char*)ytServer);
		if (upcomingLive.request()) {
			if (upcomingLive.isExist()) {
				// サムネイルを取得,表示
				thumbnail.setup((char*)upcomingLive.getThumbnailUrl(), (char*)imageServer);
				if (thumbnail.request()) {
					lcd.drawJpg(thumbnail.getJpgData(), thumbnail.getImgLength(), 0, 0, thumbnail.getImgWidth(), thumbnail.getImgHeight());
				}
				// 配信開始時刻を取得
				String ytVideoApiUrl = String(ytVideoApiUrlNoIdNoKey) + String(upcomingLive.getLiveId()) + "&key=" +  String(apiKey);
				liveStartTime.setup((char*)ytVideoApiUrl.c_str(), (char*)ytServer);
				if (liveStartTime.request()) {
					scheduledStartTime = liveStartTime.getStartTime();
				}
				// カウントダウンモードへ移行
				if (thumbnail.isSucceeded() && liveStartTime.isSucceeded()) mode = 1;
			}
		}
	} else if (mode == 1){
		// 配信までのカウントダウン
		displayTimer();
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