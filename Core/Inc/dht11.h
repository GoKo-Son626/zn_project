#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

// 对应你要求的 PG11
#define DHT11_GPIO_PORT  GPIOG
#define DHT11_GPIO_PIN   GPIO_PIN_11

// 使用 HAL 库方式实现 IO 操作
#define DHT11_DQ_OUT(n)  HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, (GPIO_PinState)n)
#define DHT11_DQ_IN      HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN)

uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#endif
