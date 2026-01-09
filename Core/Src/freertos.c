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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Pv */
// 1. 存放 ADC DMA 搬运来的原始数据 (对应 IN10, IN11, IN12)
uint16_t adc_raw_data[3] = {0};

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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t oledTaskHandle;
void StartGUITask(void *argument);
osThreadId_t dht11TaskHandle;
void StartDHT11Task(void *argument);
osThreadId_t motorTaskHandle;
void StartMotorTask(void *argument);
osThreadId_t visionTaskHandle;
void StartVisionTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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
  visionTaskHandle = osThreadNew(StartVisionTask, NULL, &defaultTask_attributes);

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
  OLED_ShowString(8, 0, (u8*)"Indoor Monitor", 16); 
  
  // 2. 静态标签：改用 12 点阵，节省空间
  OLED_ShowString(0, 20, (u8*)"Temp:", 12); // y=20
  OLED_ShowString(0, 32, (u8*)"Humi:", 12); // y=32
  
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

    osDelay(2500); // DHT11 必须 2 秒以上读一次
  }
}

void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN StartMotorTask */
  Motor_Init_All(); // 初始化点击驱动模块
  
  for(;;)
  {
    // 逻辑判定：假设温度阈值为 27 度
    if(temperature >= 28)
    {
        // 1. 马达以 20% 速度正转（作为散热风扇）
        Motor_Set(20, 1);
    } else if ( (temperature >= 26) || (res_vision == HAL_OK) ){
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

/**
  * @brief  视觉监控任务：通过检测 USART3 数据流判定人脸识别状态
  */
void StartVisionTask(void *argument)
{
  uint8_t dummy_data;

  /* 串口3接收缓冲区，用于简单观察数据 */
  for(;;)
  {
    /* 尝试从 USART3 (接ESP32) 接收 1 个字节 [cite: 313] */
    /* 商家固件在检测到人脸时会持续输出字符串 [cite: 269, 319] */
    res_vision = HAL_UART_Receive(&huart3, &dummy_data, 1, 10);

    if (res_vision == HAL_OK) 
    {
      // 1. 联动报警逻辑：发现数据流即代表检测到人脸 [cite: 8, 319]
      // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); 

      // 2. 串口打印输出到 USART1 (笔记本端查看) [cite: 121]
      // 使用 %c 可以直接看到 ESP32 发过来的字符内容（如 'c', 'e', 'n' 等）
      printf("[STM32] Detect Face! Byte Received: %c (Val: %d)\r\n", dummy_data, dummy_data);
      
      // 延时 500ms 避免打印信息过快刷屏
      osDelay(500); 
    }
    else 
    {
      // 未接收到数据，说明当前无识别结果 [cite: 268]
    }

    // 给系统留出空隙，防止任务过载
    osDelay(50); 
  }
}
// /**
//   * @brief  视觉监控任务：简单判断 USART3 是否有数据流
//   */
// void StartVisionTask(void *argument)
// {
//   uint8_t dummy_data;
//
//   for(;;)
//   {
//     /* 尝试从 USART3 接收 1 个字节，超时时间设短一点（10ms） */
//     /* 只要 res 返回 HAL_OK，就代表 ESP32 的串口有输出，即检测到人脸 */
//     res_vision = HAL_UART_Receive(&huart3, &dummy_data, 1, 10);
//
//     if (res_vision == HAL_OK) 
//     {
//       // HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET); // 蜂鸣器响
//
//       printf("[STM32] Detect Face! Raw Data............................: %d\r\n", dummy_data);
//
//       osDelay(500); 
//     }
//     else 
//     {
//     }
//
//     // 给系统留出空隙，防止任务过载
//     osDelay(50); 
//   }
// }
