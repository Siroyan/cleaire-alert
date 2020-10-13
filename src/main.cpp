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
#include "CountdownTimer.h"
#include "time.h"

#define M_IMG_WIDTH 320
#define M_IMG_HEIGHT 180

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

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

CountdownTimer cTimer;

void setup() {
	M5.begin();

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

	// NTP
	configTime(0, 0, ntpServer);
}

void loop() {
	M5.update();
	if (mode == 0 && M5.BtnA.wasPressed()) {
		String ytSearchApiUrl = String(ytSearchApiUrlNoKey) + String(apiKey);
		upcomingLive.setup((char*)ytSearchApiUrl.c_str(), (char*)ytServer);
		if (upcomingLive.request()) {
			Serial.println("Request succeeded.");
			if (upcomingLive.isExist()) {
				Serial.println("Upcoming Live is exist.");
				// サムネイルを取得,表示
				thumbnail.setup((char*)upcomingLive.getThumbnailUrl(), (char*)imageServer);
				if (thumbnail.request()) lcd.drawJpg(thumbnail.getJpgData(), thumbnail.getImgLength(), 0, 0, M_IMG_WIDTH, M_IMG_HEIGHT);
				// 配信開始時刻を取得
				String ytVideoApiUrl = String(ytVideoApiUrlNoIdNoKey) + String(upcomingLive.getLiveId()) + "&key=" +  String(apiKey);
				liveStartTime.setup((char*)ytVideoApiUrl.c_str(), (char*)ytServer);
				// 配信開始時刻を取得
				if (liveStartTime.request()) scheduledStartTime = liveStartTime.getStartTime();
				// カウントダウンモードへ移行
				if (thumbnail.isSucceeded() && liveStartTime.isSucceeded()) mode = 1;
			}
		}
	} else if (mode == 1) {
		// タイマーをセット
		cTimer.update(scheduledStartTime, time(NULL));
		// タイマーを表示
		if (cTimer.isSoon()) lcd.drawString("開始まで " + cTimer.getDiffH() + ":" + cTimer.getDiffM() + ":" + cTimer.getDiffS(), 5, M_IMG_HEIGHT + 10);
		else lcd.drawString("一日以上先の配信です", 10, M_IMG_HEIGHT + 10);
	}
}