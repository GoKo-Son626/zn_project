#ifndef __OLED_H
#define __OLED_H 

#include "main.h" 
#include "i2c.h" // 引入CubeMX生成的i2c头文件

// ----------------- OLED I2C 地址 -----------------
// 通常 0.96寸 OLED 的地址是 0x78
#define OLED_ADDR  0x78

// ----------------- 宏定义 -----------------
#define OLED_CMD  0 // 写命令
#define OLED_DATA 1 // 写数据

// 为了兼容你原来的代码，保留 u8 u32 定义
#ifndef u8
#define u8 uint8_t
#endif

#ifndef u32
#define u32 uint32_t
#endif

// ----------------- 函数声明 -----------------
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1);
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1);
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1);
void OLED_ShowPicture(u8 x0,u8 y0,u8 x1,u8 y1,u8 BMP[]);
void OLED_Refresh(void); // 别忘了这个刷新函数，显存操作的核心

// 绘图函数
void OLED_DrawPoint(u8 x,u8 y);
void OLED_ClearPoint(u8 x,u8 y);
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2);
void OLED_DrawCircle(u8 x,u8 y,u8 r);

// 反显等功能
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);

#endif
