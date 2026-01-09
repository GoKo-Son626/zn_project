#ifndef __HEATER_H
#define __HEATER_H

#include "main.h"

// 1. 修改端口为 GPIOC
#define HEATER_PORT GPIOC
// 2. 修改引脚为 GPIO_PIN_13
#define HEATER_PIN  GPIO_PIN_13

// 状态定义
#define HEATER_ON   1
#define HEATER_OFF  0

// 函数声明
void Heater_Init(void);
void Heater_Set_State(uint8_t state);

#endif
