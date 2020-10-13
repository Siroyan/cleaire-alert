#include <Arduino.h>
#include "HttpCommunication.h"
#include "Thumbnail.h"

uint8_t* Thumbnail::getJpgData() {
    String recievedImgString = HttpCommunication::getRecievedData();
    uint16_t jpgDataLength = recievedImgString.length();
    
    // サムネ画像のバイナリデータを作成
    uint8_t* jpgData = new uint8_t[jpgDataLength];
    for (int i=0; i < jpgDataLength; i++) jpgData[i] = recievedImgString.charAt(i);
    return jpgData;
}

uint16_t Thumbnail::getImgLength() {
    String recievedImgString = HttpCommunication::getRecievedData();
    return recievedImgString.length();
}