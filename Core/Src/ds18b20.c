#include "ds18b20.h"

// =================================================================
// 终极精准延时：使用 DWT 硬件计数器
// =================================================================
void delay_us(uint32_t us)
{
    uint32_t t0 = DWT->CYCCNT;
    // 72MHz 主频下，1us = 72 个时钟周期
    uint32_t delta = us * 72;
    
    // 循环等待，直到计数器的差值达到 delta
    // 这种写法不用担心计数器溢出归零，无符号减法会自动处理
    while ((DWT->CYCCNT - t0) < delta);
}

// =================================================================
// 寄存器操作宏 (PG11)
// =================================================================

// 切换为：输入模式 (开启内部上拉，CNF=10, ODR=1)
static __inline void DS18B20_Mode_IPU(void) {
    GPIOG->CRH &= 0xFFFF0FFF; // 清除配置
    GPIOG->CRH |= 0x00008000; // CNF=10 (输入上拉/下拉), MODE=00 (输入)
    GPIOG->ODR |= GPIO_PIN_11;// ODR=1 (上拉)
}

// 切换为：推挽输出模式 (CNF=00, MODE=11 -> 50MHz)
static __inline void DS18B20_Mode_Out_PP(void) {
    GPIOG->CRH &= 0xFFFF0FFF; // 清除配置
    GPIOG->CRH |= 0x00003000; // CNF=00 (推挽), MODE=11 (50MHz)
}

// 拉高拉低宏
#define DQ_HIGH() (GPIOG->BSRR = GPIO_PIN_11)
#define DQ_LOW()  (GPIOG->BSRR = (uint32_t)GPIO_PIN_11 << 16)
#define DQ_READ() ((GPIOG->IDR & GPIO_PIN_11) ? 1 : 0)

// =================================================================
// 时序逻辑
// =================================================================

// 复位
static void DS18B20_Rst(void)
{
    DS18B20_Mode_Out_PP();     
    DQ_LOW();
    delay_us(750);            // 精准 750us
    DQ_HIGH();
    delay_us(15);             // 精准 15us
}

// 检测存在
static uint8_t DS18B20_Presence(void)
{
    uint8_t pulse_time = 0;
    DS18B20_Mode_IPU();       

    // 等待拉低 (最大 100us)
    while( DQ_READ() && pulse_time<100 )
    {
        pulse_time++;
        delay_us(1);
    }   
    if( pulse_time >=100 ) return 1;
    
    pulse_time = 0;
    
    // 等待释放 (最大 240us)
    while( !DQ_READ() && pulse_time<240 )
    {
        pulse_time++;
        delay_us(1);
    }   
    if( pulse_time >=240 ) return 1;
    
    return 0;
}

// 读一位 (核心难点)
static uint8_t DS18B20_Read_Bit(void)
{
    uint8_t dat;
    
    // 1. 启动时序：拉低 >1us
    DS18B20_Mode_Out_PP();
    DQ_LOW();
    delay_us(2); 
    
    // 2. 释放总线，切换输入，让内部上拉电阻把电平拉高
    DS18B20_Mode_IPU(); 
    
    // 3. 【关键调整】
    // 有电阻时，通常延时 8-10us 就读。
    // 没电阻时，内部上拉很弱，电平爬升慢，我们多等一会儿，等到 13us 再读。
    // 注意：绝对不能超过 15us，否则就读到下一位了。
    delay_us(13); 
    
    // 4. 读取
    if( DQ_READ() ) dat = 1;
    else dat = 0;
    
    // 5. 补足 60us 时间槽
    delay_us(50); 
    return dat;
}

// 读字节
uint8_t DS18B20_Read_Byte(void)
{
    uint8_t i, j, dat = 0;
    for(i=0; i<8; i++) 
    {
        j = DS18B20_Read_Bit();
        dat = (dat) | (j<<i);
    }
    return dat;
}

// 写字节
void DS18B20_Write_Byte(uint8_t dat)
{
    uint8_t i, testb;
    DS18B20_Mode_Out_PP();
    
    for( i=0; i<8; i++ )
    {
        testb = dat&0x01;
        dat = dat>>1;       
        if (testb) // 写1
        {           
            DQ_LOW();
            delay_us(2);      
            DQ_HIGH();
            delay_us(60);     
        }       
        else // 写0
        {           
            DQ_LOW();  
            delay_us(60);
            DQ_HIGH();          
            delay_us(2);
        }
    }
}

// 初始化
uint8_t DS18B20_Init(void)
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    DS18B20_Mode_Out_PP();
    DQ_HIGH();
    
    DS18B20_Rst();
    return DS18B20_Presence();
}

// 获取温度
float DS18B20_Get_Temp(void)
{
    uint8_t tpmsb, tplsb;
    int16_t s_tem;
    float f_tem;
    
    DS18B20_Rst();      
    DS18B20_Presence();  
    DS18B20_Write_Byte(0XCC); 
    DS18B20_Write_Byte(0X44); 
    
    // 在无电阻模式下，转换期间最好不要操作总线
    // 我们直接进行下一次复位
    
    DS18B20_Rst();
    DS18B20_Presence();
    DS18B20_Write_Byte(0XCC); 
    DS18B20_Write_Byte(0XBE); 
    
    tplsb = DS18B20_Read_Byte();        
    tpmsb = DS18B20_Read_Byte(); 
    
    // 组合数据
    s_tem = (int16_t)((tpmsb << 8) | tplsb);
    
    // 过滤掉错误的 0 值 (如果读出来是0，说明可能还是没读对，保留上次的值)
    if(s_tem == 0) return 0.0f; 
    
    // 计算温度
    f_tem = s_tem * 0.0625f;
      
    return f_tem;   
}
