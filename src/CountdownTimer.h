#ifndef COUNTDOWNTIMER_H_INCLUDE
#define COUNTDOWNTIMER_H_INCLUDE

#include <Arduino.h>

class CountdownTimer {
    public:
        CountdownTimer();
        void update(const char* _startTime, long _nowTime);
        bool isUpcoming();
        bool isSoon();
        String getDiffH();
        String getDiffM();
        String getDiffS();
    private:
        long diffTime;
        long nowTime;
        const char* startTime;
};

#endif