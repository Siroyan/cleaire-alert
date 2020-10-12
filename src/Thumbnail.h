#ifndef THUMBNAIL_H_INCLUDE
#define THUMBNAIL_H_INCLUDE

#include <Arduino.h>
#include "HttpCommunication.h"

class Thumbnail : public HttpCommunication {
    public:
        uint8_t* getJpgData();
        uint16_t getImgLength();
        uint8_t getImgWidth();
        uint8_t getImgHeight();
};

#endif