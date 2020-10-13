#include <Arduino.h>
#include "CountdownTimer.h"

CountdownTimer::CountdownTimer() {

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

void CountdownTimer::update(const char* _startTime, long _nowTime) {
    startTime = _startTime;
    nowTime = _nowTime;
    diffTime = convertToUnixTime(startTime) - nowTime;
}

bool CountdownTimer::isSoon() {
    if (diffTime > 24 * 60 * 60) return false;
    else return true;
}

String CountdownTimer::getDiffH() {
    uint8_t diffH = diffTime / 3600;
    return diffH < 10 ? "0" + String(diffH) : String(diffH);
}

String CountdownTimer::getDiffM() {
    uint8_t diffM = (diffTime % 3600) / 60;
    return diffM < 10 ? "0" + String(diffM) : String(diffM);
}

String CountdownTimer::getDiffS() {
    uint8_t diffS = diffTime % 60;
    return diffS < 10 ? "0" + String(diffS) : String(diffS);
}