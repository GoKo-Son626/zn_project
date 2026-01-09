#include "esp32_cam.h"
#include "i2c.h" 

uint8_t ESP32_CAM_PollFace(FaceData_t *face) {
    uint8_t buf[4] = {0};
    uint8_t reg = 0x01; // 寄存器地址

    // 使用 hi2c2 询问 ESP32 (地址 0x52)
    // 注意：HAL库地址要左移一位，即 0x52 << 1 = 0xA4
    if (HAL_I2C_Mem_Read(&hi2c2, 0xA4, reg, I2C_MEMADD_SIZE_8BIT, buf, 4, 100) == HAL_OK) {
        face->center_x = buf[0];
        face->center_y = buf[1];
        face->width    = buf[2];
        face->length   = buf[3];

        if (face->width > 0 && face->length > 0) {
            face->is_detected = 1;
            return 1;
        }
    }
    face->is_detected = 0;
    return 0;
}
