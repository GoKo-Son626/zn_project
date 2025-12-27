#include "dht11.h"
#include <stdio.h>

// 使用你 main 里的 DWT 实现微秒延时
static void delay_us(uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - startTick) < delayTicks);
}

// 设置 PG11 为输出
void DHT11_IO_OUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin = DHT11_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

// 设置 PG11 为输入
void DHT11_IO_IN(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin = DHT11_GPIO_PIN;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLUP; // 建议上拉，防止浮空
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

void DHT11_Rst(void) {
    DHT11_IO_OUT();
    DHT11_DQ_OUT(0);  // 拉低
    HAL_Delay(20);    // HAL的ms延时
    DHT11_DQ_OUT(1);  // 拉高
    delay_us(30);
}

uint8_t DHT11_Check(void) {
    uint8_t retry = 0;
    DHT11_IO_IN();
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;
    else retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    if (retry >= 100) return 1;
    return 0;
}

uint8_t DHT11_Read_Bit(void) {
    uint8_t retry = 0;
    while (DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    retry = 0;
    while (!DHT11_DQ_IN && retry < 100) {
        retry++;
        delay_us(1);
    }
    delay_us(40);
    if (DHT11_DQ_IN) return 1;
    else return 0;
}

uint8_t DHT11_Read_Byte(void) {
    uint8_t i, dat = 0;
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi) {
    uint8_t buf[5];
    DHT11_Rst();
    if (DHT11_Check() == 0) {
        for (uint8_t i = 0; i < 5; i++) {
            buf[i] = DHT11_Read_Byte();
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *humi = buf[0];
            *temp = buf[2];
            return 0; // 成功
        }
    }
    return 1; // 失败
}

uint8_t DHT11_Init(void) {
    DHT11_Rst();
    return DHT11_Check();
}
