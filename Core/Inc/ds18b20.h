#ifndef __DS18B20_H
#define __DS18B20_H

#include "main.h" // 必须包含这个，否则不认 HAL 库

// --------------- 引脚配置 (修改为你的 PG11) ---------------
#define DS18B20_PORT      GPIOG
#define DS18B20_PIN       GPIO_PIN_11

// --------------- 宏定义替换 (HAL库版本) ---------------
// 拉高/拉低
#define DS18B20_DATA_OUT(a)  HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, (a) ? GPIO_PIN_SET : GPIO_PIN_RESET)

// 读取引脚电平
#define DS18B20_DATA_IN()    HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)

// --------------- 函数声明 (保持原样) ---------------
uint8_t DS18B20_Init(void);
float DS18B20_Get_Temp(void);

#endif
