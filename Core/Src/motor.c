#include "motor.h"
#include "tim.h"  // 必须包含 tim.h 以使用 htim3

// 初始化马达相关的 GPIO
static void Motor_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOG_CLK_ENABLE();

    // 配置 PG10 和 PG12 为推挽输出
    GPIO_InitStruct.Pin = MOTOR_AIN1_PIN | MOTOR_AIN2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    // 默认停止
    HAL_GPIO_WritePin(GPIOG, MOTOR_AIN1_PIN | MOTOR_AIN2_PIN, GPIO_PIN_RESET);
}

void Motor_Init_All(void) {
    Motor_GPIO_Init();
    // 启动 TIM3 的 PWM 通道 1 (PA6)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    // 确保 STBY 拉高
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_14, GPIO_PIN_SET);
}

/**
 * @param speed: 0-100 (百分比速度)
 * @param forward: 1正转, 0反转
 */
void Motor_Set(int speed, uint8_t forward) {
    if (speed > 100) speed = 100;
    
    // 计算比较值： (speed / 100.0) * (Period + 1)
    // 这里 Period 是 19999，所以 100% 对应 20000
    uint32_t compare_value = (uint32_t)(speed * 10);

    if (forward) {
        HAL_GPIO_WritePin(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN, GPIO_PIN_SET);
    }
    
    // 更新 TIM3 通道 1 的占空比
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare_value);
}

void Motor_Stop(void) {
    HAL_GPIO_WritePin(MOTOR_AIN1_PORT, MOTOR_AIN1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_AIN2_PORT, MOTOR_AIN2_PIN, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
}
