#include "heater.h"

// 初始化函数 (其实 MX_GPIO_Init 已经做了，这里是为了逻辑完整)
void Heater_Init(void)
{
    // 确保初始化时是关闭状态
    Heater_Set_State(HEATER_OFF);
}

// 控制函数
// state: 1=加热, 0=停止
void Heater_Set_State(uint8_t state)
{
    if(state == HEATER_ON)
    {
        // 输出高电平 -> MOSFET导通 -> 加热棒工作
        HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_SET);
    }
    else
    {
        // 输出低电平 -> MOSFET截止 -> 停止加热
        HAL_GPIO_WritePin(HEATER_PORT, HEATER_PIN, GPIO_PIN_RESET);
    }
}
