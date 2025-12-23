#include "oled.h"
#include "oledfont.h"  // 务必保证你的工程里有这个文件，并且路径包含了它
#include "string.h"    // 用于 memset

// 显存数组
u8 OLED_GRAM[144][8];

// -------------------------------------------------------------------------
// 核心底层函数：向 OLED 发送一个字节
// mode: OLED_CMD (命令) 或 OLED_DATA (数据)
// -------------------------------------------------------------------------
void OLED_WR_Byte(u8 dat, u8 mode)
{
    // 使用 HAL 库的 内存写模式 直接发送
    // 0x00 是命令寄存器地址，0x40 是数据寄存器地址
    HAL_I2C_Mem_Write(&hi2c1, OLED_ADDR, (mode == OLED_CMD) ? 0x00 : 0x40, 
                      I2C_MEMADD_SIZE_8BIT, &dat, 1, 10);
}

// -------------------------------------------------------------------------
// 初始化函数 (已移除 GPIO 初始化，只保留 OLED 寄存器配置)
// -------------------------------------------------------------------------
void OLED_Init(void)
{
    // 注意：这里不需要初始化 GPIO 和 I2C，因为 CubeMX 生成的 main.c 里的 MX_I2C1_Init 已经做完了
    
    HAL_Delay(200); // 上电延时，让屏幕准备好

    OLED_WR_Byte(0xAE,OLED_CMD); // --turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD); // ---set low column address
    OLED_WR_Byte(0x10,OLED_CMD); // ---set high column address
    OLED_WR_Byte(0x40,OLED_CMD); // --set start line address
    OLED_WR_Byte(0x81,OLED_CMD); // --set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD); // Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD); // --Set SEG/Column Mapping
    OLED_WR_Byte(0xC8,OLED_CMD); // Set COM/Row Scan Direction
    OLED_WR_Byte(0xA6,OLED_CMD); // --set normal display
    OLED_WR_Byte(0xA8,OLED_CMD); // --set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD); // --1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD); // -set display offset
    OLED_WR_Byte(0x00,OLED_CMD); // -not offset
    OLED_WR_Byte(0xd5,OLED_CMD); // --set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD); // --set divide ratio
    OLED_WR_Byte(0xD9,OLED_CMD); // --set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD); // 
    OLED_WR_Byte(0xDA,OLED_CMD); // --set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD); // --set vcomh
    OLED_WR_Byte(0x40,OLED_CMD); // 
    OLED_WR_Byte(0x20,OLED_CMD); // -Set Page Addressing Mode
    OLED_WR_Byte(0x02,OLED_CMD); // 
    OLED_WR_Byte(0x8D,OLED_CMD); // --set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD); // --set(0x10) disable
    OLED_WR_Byte(0xA4,OLED_CMD); // Disable Entire Display On
    OLED_WR_Byte(0xA6,OLED_CMD); // Disable Inverse Display On
    OLED_WR_Byte(0xAF,OLED_CMD); // --turn on oled panel
    
    OLED_Clear(); // 初始清屏
}

// -------------------------------------------------------------------------
// 以下是图形逻辑函数 (基本不用动，逻辑是通用的)
// -------------------------------------------------------------------------

void OLED_ColorTurn(u8 i)
{
    if(i==0) OLED_WR_Byte(0xA6,OLED_CMD);
    if(i==1) OLED_WR_Byte(0xA7,OLED_CMD);
}

void OLED_DisplayTurn(u8 i)
{
    if(i==0) { OLED_WR_Byte(0xC8,OLED_CMD); OLED_WR_Byte(0xA1,OLED_CMD); }
    if(i==1) { OLED_WR_Byte(0xC0,OLED_CMD); OLED_WR_Byte(0xA0,OLED_CMD); }
}

void OLED_Refresh(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte(0xb0+i,OLED_CMD); 
        OLED_WR_Byte(0x00,OLED_CMD);   
        OLED_WR_Byte(0x10,OLED_CMD);   
        // 优化：利用硬件I2C连续写入特性，提高刷屏速度
	for(n=0; n < 128; n++)
	{
		OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
	}
    }
}

void OLED_Clear(void)
{
    u8 i,n;
    // 使用 memset 快速清零显存
    memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
    OLED_Refresh();
}

void OLED_DrawPoint(u8 x,u8 y)
{
    u8 i,m,n;
    if(x>127||y>63) return; // 防止越界
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]|=n;
}

void OLED_ClearPoint(u8 x,u8 y)
{
    u8 i,m,n;
    if(x>127||y>63) return;
    i=y/8;
    m=y%8;
    n=1<<m;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
    OLED_GRAM[x][i]|=n;
    OLED_GRAM[x][i]=~OLED_GRAM[x][i];
}

void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2)
{
    u8 i,k,k1,k2,y0;
    if((x1<0)||(x2>128)||(y1<0)||(y2>64)||(x1>x2)||(y1>y2))return;
    if(x1==x2)    
    {
        for(i=0;i<(y2-y1);i++) OLED_DrawPoint(x1,y1+i);
    }
    else if(y1==y2)   
    {
        for(i=0;i<(x2-x1);i++) OLED_DrawPoint(x1+i,y1);
    }
    else      
    {
        k1=y2-y1;
        k2=x2-x1;
        k=k1*10/k2;
        for(i=0;i<(x2-x1);i++) OLED_DrawPoint(x1+i,y1+i*k/10);
    }
}

void OLED_DrawCircle(u8 x,u8 y,u8 r)
{
    int a, b,num;
    a = 0; b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b);
        OLED_DrawPoint(x - a, y - b);
        OLED_DrawPoint(x - a, y + b);
        OLED_DrawPoint(x + a, y + b);
        OLED_DrawPoint(x + b, y + a);
        OLED_DrawPoint(x + b, y - a);
        OLED_DrawPoint(x - b, y - a);
        OLED_DrawPoint(x - b, y + a);
        a++;
        num = (a * a + b * b) - r*r;
        if(num > 0) { b--; a--; }
    }
}

void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1)
{
    u8 i,m,temp,size2,chr1;
    u8 y0=y;
    size2=(size1/8+((size1%8)?1:0))*(size1/2); 
    chr1=chr-' '; 
    for(i=0;i<size2;i++)
    {
        // 注意：这里需要 oledfont.h 文件支持
        if(size1==12)      {temp=asc2_1206[chr1][i];} 
        else if(size1==16) {temp=asc2_1608[chr1][i];} 
        else if(size1==24) {temp=asc2_2412[chr1][i];} 
        else return;
        
        for(m=0;m<8;m++)           
        {
            if(temp&0x80)OLED_DrawPoint(x,y);
            else OLED_ClearPoint(x,y);
            temp<<=1;
            y++;
            if((y-y0)==size1)
            {
                y=y0; x++; break;
            }
        }
    }
}

void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1)
{
    while((*chr>=' ')&&(*chr<='~'))
    {
        OLED_ShowChar(x,y,*chr,size1);
        x+=size1/2;
        if(x>128-size1) { x=0; y+=2; }
        chr++;
    }
}

u32 OLED_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--) { result*=m; }
    return result;
}

void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1)
{
    u8 t,temp;
    for(t=0;t<len;t++)
    {
        temp=(num/OLED_Pow(10,len-t-1))%10;
        if(temp==0) OLED_ShowChar(x+(size1/2)*t,y,'0',size1);
        else        OLED_ShowChar(x+(size1/2)*t,y,temp+'0',size1);
    }
}

void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1)
{
    u8 i,m,n=0,temp,chr1;
    u8 x0=x,y0=y;
    u8 size3=size1/8;
    while(size3--)
    {
        chr1=num*size1/8+n;
        n++;
        for(i=0;i<size1;i++)
        {
            if(size1==16)      {temp=Hzk1[chr1][i];}
            else if(size1==24) {temp=Hzk2[chr1][i];}
            else if(size1==32) {temp=Hzk3[chr1][i];}
            else if(size1==64) {temp=Hzk4[chr1][i];}
            else return;
                        
            for(m=0;m<8;m++)
            {
                if(temp&0x01)OLED_DrawPoint(x,y);
                else OLED_ClearPoint(x,y);
                temp>>=1;
                y++;
            }
            x++;
            if((x-x0)==size1) {x=x0;y0=y0+8;}
            y=y0;
         }
    }
}

void OLED_ShowPicture(u8 x0,u8 y0,u8 x1,u8 y1,u8 BMP[])
{
    u32 j=0;
    u8 x=0,y=0;
    if(y%8==0)y=0; else y+=1;
    for(y=y0;y<y1;y++)
    {
         OLED_WR_Byte(0xb0+y,OLED_CMD); 
         OLED_WR_Byte(((x0&0xf0)>>4)|0x10,OLED_CMD);
         OLED_WR_Byte((x0&0x0f),OLED_CMD);
         for(x=x0;x<x1;x++)
         {
             OLED_WR_Byte(BMP[j],OLED_DATA);
             j++;
         }
    }
}
