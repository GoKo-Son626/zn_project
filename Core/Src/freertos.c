/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "oled.h"
#include "string.h"
#include "ds18b20.h"
#include "adc.h"
#include "dht11.h"
#include "motor.h"
#include "esp32_cam.h"
#include "usart.h"
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define ESP_RX_BUF_SIZE 512
uint8_t esp_rx_buf[ESP_RX_BUF_SIZE]; // DMA直接存放数据的数组
uint8_t esp_process_buf[ESP_RX_BUF_SIZE]; // 拷贝出来处理的数组
uint16_t esp_rx_len = 0;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Pv */
// 1. 存放 ADC DMA 搬运来的原始数据 (对应 IN10, IN11, IN12)
uint16_t adc_raw_data[3] = {0}; // [0]:MQ135, [1]:假设的传感器2, [2]:假设的传感器3

float mq135_voltage = 0;        // MQ135 转换后的电压值
uint16_t air_quality = 0;       // 模拟空气质量数值

float Light_voltage = 0;
uint16_t light_intensity = 0;

// 2. 存放处理后的物理量 (给 GUI 显示用)
uint8_t temperature, humidity;

HAL_StatusTypeDef res_vision;
/* USER CODE END Pv */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* --- esp8266任务属性 (给 2KB 栈空间) --- */
const osThreadAttr_t heaterTask_attributes = {
  .name = "communication",
  .stack_size = 512 * 4, // 改成 512 Words (2048 Bytes)
  .priority = (osPriority_t) osPriorityNormal,
};
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t oledTaskHandle;
void StartGUITask(void *argument);
osThreadId_t dht11TaskHandle;
void StartDHT11Task(void *argument);
osThreadId_t motorTaskHandle;
void StartMotorTask(void *argument);
/* USER CODE END FunctionPrototypes */
osThreadId_t mq135TaskHandle;
void StartMQ135Task(void *argument);
osThreadId_t communicationTaskHandle;
void StartCommunicationTask(void *argument);
void StartDefaultTask(void *argument);
osThreadId_t LightTaskHandle;
void StartLightTask(void *argument);
void Servo_SetAngle(float angle);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
}
/* USER CODE END 5 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  oledTaskHandle = osThreadNew(StartGUITask, NULL, &defaultTask_attributes);
  dht11TaskHandle = osThreadNew(StartDHT11Task, NULL, &defaultTask_attributes);
  motorTaskHandle = osThreadNew(StartMotorTask, NULL, &defaultTask_attributes);
  mq135TaskHandle = osThreadNew(StartMQ135Task, NULL, &defaultTask_attributes);
  communicationTaskHandle = osThreadNew(StartCommunicationTask, NULL, &heaterTask_attributes);
  LightTaskHandle = osThreadNew(StartLightTask, NULL, &defaultTask_attributes);
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  printf("System start success!\r\n");
  /* Infinite loop */
  for(;;)
  {
    // osDelay(1);
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    osDelay(1000);
    // printf("System Alive! Tick: %lu\r\n", HAL_GetTick());

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartGUITask(void *argument)
{
  OLED_Init();
  OLED_Clear();
  
  // 1. 标题：使用 16 点阵，占据 y=0 到 y=15 的位置
  OLED_ShowString(8, 0, (u8*)"Lab Monitor", 16); 
  
  // 2. 静态标签：改用 12 点阵，节省空间
  OLED_ShowString(0, 20, (u8*)"Temp:", 12); // y=20
  OLED_ShowString(0, 32, (u8*)"Humi:", 12); // y=32
  OLED_ShowString(0, 44, (u8*)"Air:", 12); // y=32
  OLED_ShowString(0, 56, (u8*)"Light:", 12); // y=32
  
  // --- 这里 y=44 和 y=56 的位置现在空着，留给你后续的传感器 ---
  
  OLED_Refresh();

  char disp_buf[16];

  for(;;)
  {
    // --- 刷新温度值 (x轴 40 像素开始，避开标签) ---
    sprintf(disp_buf, "%d C   ", temperature);
    OLED_ShowString(40, 20, (u8*)disp_buf, 12);

    // --- 刷新湿度值 ---
    sprintf(disp_buf, "%d %%   ", humidity);
    OLED_ShowString(40, 32, (u8*)disp_buf, 12);

    // --- 刷新 MQ135 值 ---
    sprintf(disp_buf, "%d      ", air_quality); // 后面加空格是为了清除旧数字的残影
    OLED_ShowString(40, 44, (u8*)disp_buf, 12);

    sprintf(disp_buf, "%d      ", light_intensity); // 后面加空格是为了清除旧数字的残影
    OLED_ShowString(40, 56, (u8*)disp_buf, 12);

    // --- 系统运行指示点 (右上角) ---
    static uint8_t tick = 0;
    if(tick) OLED_ShowString(120, 0, (u8*)".", 16);
    else     OLED_ShowString(120, 0, (u8*)" ", 16);
    tick = !tick;

    OLED_Refresh();
    osDelay(500);
  }
}

void StartDHT11Task(void *argument)
{
  
  // 核心：在OS开始后先初始化引脚时钟
  __HAL_RCC_GPIOG_CLK_ENABLE(); 

  for(;;)
  {
    // 读取数据
    if(DHT11_Read_Data(&temperature, &humidity) == 0)
    {
      // printf("DHT11 OK! Humidity: %d%%, Temperature: %d C\r\n", humidity, temperature);
    }
    else
    {
      // printf("DHT11 Error! Check wiring on PG11\r\n");
    }

    osDelay(1000); // DHT11 必须 2 秒以上读一次
  }
}

void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN StartMotorTask */
  Motor_Init_All(); // 初始化点击驱动模块
  
  for(;;)
  {
    // 逻辑判定：假设温度阈值为 27 度
    if(temperature >= 32)
    {
        // 1. 马达以 20% 速度正转（作为散热风扇）
        Motor_Set(20, 1);
        // 2. 关闭蜂鸣器 (PG13 高电平关闭)
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
    } else if (temperature >= 30){
        // 2. 蜂鸣器报警 (PG13 低电平触发)
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
        Motor_Stop();
    } else {
        // 1. 停止马达
        Motor_Stop();
        // 2. 关闭蜂鸣器 (PG13 高电平关闭)
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
    }

    osDelay(1000); // 1秒轮询一次
  }
  /* USER CODE END StartMotorTask */
}

// MQ135 ADC转换
void StartMQ135Task(void *argument)
{
  for(;;)
  {
    /* 根据你的配置：
       PC0 (IN10) -> adc_raw_data[0]
       PC1 (IN11) -> adc_raw_data[1]
       PC2 (IN12) -> adc_raw_data[2]
    */
    
    // 1. 将 12 位 ADC 值 (0-4095) 转换为电压 (0-3.3V)
    mq135_voltage = (float)adc_raw_data[0] * 3.3f / 4096.0f;
    
    // 2. 简单的换算（可选）：将 0-4095 映射到 0-100 的空气质量指数
    air_quality = (uint16_t)((float)adc_raw_data[0] / 4095.0f * 100.0f);
    
    // 打印调试
    // printf("MQ135 Raw: %d, Voltage: %.2fV\r\n", adc_raw_data[0], mq135_voltage);

    osDelay(500); // 半秒计算一次即可
  }
}

// light ADC转换
void StartLightTask(void *argument)
{
  for(;;)
  {
    /* 根据你的配置：
       PC0 (IN10) -> adc_raw_data[0]
       PC1 (IN11) -> adc_raw_data[1]
       PC2 (IN12) -> adc_raw_data[2]
    */
    
    // 1. 将 12 位 ADC 值 (0-4095) 转换为电压 (0-3.3V)
    Light_voltage = (float)adc_raw_data[1] * 3.3f / 4096.0f;
    
    // 2. 简单的换算（可选）：将 0-4095 映射到 0-100 的空气质量指数
	light_intensity = (uint16_t)((float)(4095 - adc_raw_data[1]) / 4095.0f * 100.0f);
    
    // 打印调试
    // printf("Light Raw: %d, Voltage: %.2fV\r\n", adc_raw_data[1], Light_voltage);

    osDelay(500); // 半秒计算一次即可
  }
}

/* 辅助发送函数 */
void ESP_Send_Cmd(char *cmd) {
    printf("Sending: %s\r\n", cmd); // 在USART1打印调试信息
    HAL_UART_Transmit(&huart3, (uint8_t*)cmd, strlen(cmd), 100);
    HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 10);
}

// 通信初始化函数（内部调用）
void ESP_Init_Connection(void) {
    printf("Starting ESP8266 Initialization...\r\n");
    
    osDelay(5000);
    // 1. 测试指令
    ESP_Send_Cmd("AT");
    osDelay(1000);
	printf("ESP Response: %s\r\n", esp_process_buf);
    
    // 2. 设置 Station 模式
    ESP_Send_Cmd("AT+CWMODE=1");
    osDelay(1000);
	printf("ESP Response: %s\r\n", esp_process_buf);
    
    // 3. 连接 WiFi (请替换为你自己的 WiFi 名和密码)
    // 注意：C语言中双引号前要加反斜杠转义 \"
    ESP_Send_Cmd("AT+CWJAP=\"HiwonderESP\",\"hiwonder\"");
    osDelay(10000); // 连接WiFi需要较长时间
	printf("ESP Response: %s\r\n", esp_process_buf);
    
    // 4. 连接笔记本 TCP 服务器 (请替换为你笔记本的局域网 IP)
    // 端口号必须是 8080，与 Python 代码对应
    ESP_Send_Cmd("AT+CIPSTART=\"TCP\",\"192.168.237.66\",8080");
    osDelay(2000);
	printf("ESP Response: %s\r\n", esp_process_buf);
    
    // 5. 开启透传模式
    ESP_Send_Cmd("AT+CIPMODE=1");
    osDelay(500);
	printf("ESP Response: %s\r\n", esp_process_buf);
    ESP_Send_Cmd("AT+CIPSEND");
    osDelay(500);
	printf("ESP Response: %s\r\n", esp_process_buf);
    
    printf("ESP8266 Link Ready!\r\n");
}

void StartCommunicationTask(void *argument) {
    osDelay(3000); // 等待系统稳定
    HAL_UART_Receive_DMA(&huart3, esp_rx_buf, ESP_RX_BUF_SIZE);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    
    ESP_Init_Connection(); // 内部会处理 AT 指令
   
    char wifi_tx_buf[128];
    for(;;) {
        // 构造发送给笔记本的数据包
        // 对应你现在的变量：temperature, humidity, air_quality
        sprintf(wifi_tx_buf, "Temp:%d, Humi:%d, Air:%d, Light:%d\n",
                temperature, humidity, air_quality, light_intensity);
        
        HAL_UART_Transmit(&huart3, (uint8_t*)wifi_tx_buf, strlen(wifi_tx_buf), 100);
        
        // 接收逻辑 (保持不变)
        if(esp_rx_len > 0) {
            printf("Cmd From PC: %s\r\n", esp_process_buf);
            // 这里可以加逻辑，比如收到 'S' 就让马达停止
            if(strstr((char*)esp_process_buf, "SERVO_TOGGLE")) {
		          Door_Unlock_Process();
            }
            esp_rx_len = 0;
            memset(esp_process_buf, 0, ESP_RX_BUF_SIZE);
        }
        osDelay(3000); // 2秒上报一次
    }
}

/**
 * @brief  舵机角度控制
 * @param  angle: 0 到 180 度
 */
void Servo_SetAngle(float angle)
{
    if (angle > 180.0f) angle = 180.0f;
    if (angle < 0.0f) angle = 0.0f;
    
    // 50Hz下, ARR=199: 
    // 0.5ms(0度)  -> Compare = 5
    // 2.5ms(180度) -> Compare = 25
    // 换算公式: CCR = 5 + (angle / 180.0) * 20
    uint16_t compare = (uint16_t)(5.0f + (angle / 180.0f) * 20.0f);
    
    // 使用 PA7 对应的 TIM3_CHANNEL_2
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, compare);
}

/**
 * @brief  模拟门禁开门流程
 */
void Door_Unlock_Process(void)
{
    // 1. 启动 PA7 的 PWM 输出 (只需启动一次，也可放初始化里)
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); 
    
    // 2. 转动到 90 度 (开门)
    Servo_SetAngle(90.0f);
    
    // 3. 延时 3 秒 (FreeRTOS 环境下使用 osDelay)
    osDelay(3000);
    
    // 4. 转动回 0 度 (关门)
    Servo_SetAngle(0.0f);
}
