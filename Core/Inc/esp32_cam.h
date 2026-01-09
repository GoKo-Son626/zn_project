#ifndef __ESP32_CAM_H
#define __ESP32_CAM_H

#include "main.h"

// 商家定义的设备地址 (0x52 << 1 = 0xA4)
#define ESP32_S3_ADDR      0xA4 
#define REG_FACE_DATA      0x01

typedef struct {
    uint8_t center_x;
    uint8_t center_y;
    uint8_t width;
    uint8_t length;
    uint8_t is_detected; // 是否检测到人
} FaceData_t;

// 函数声明
uint8_t ESP32_CAM_PollFace(FaceData_t *face);

#endif
