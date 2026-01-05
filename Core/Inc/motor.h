#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

// 方向控制引脚
#define MOTOR_AIN1_PORT  GPIOG
#define MOTOR_AIN1_PIN   GPIO_PIN_10
#define MOTOR_AIN2_PORT  GPIOG
#define MOTOR_AIN2_PIN   GPIO_PIN_12

void Motor_Init_All(void);
void Motor_Set(int speed, uint8_t forward);
void Motor_Stop(void);

#endif
