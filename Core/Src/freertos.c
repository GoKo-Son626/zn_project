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
    printf("System Alive! Tick: %lu\r\n", HAL_GetTick());

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
      printf("DHT11 OK! Humidity: %d%%, Temperature: %d C\r\n", humidity, temperature);
    }
    else
    {
      printf("DHT11 Error! Check wiring on PG11\r\n");
    }

    osDelay(2500); // DHT11 必须 2 秒以上读一次
  }
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the motorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
  /* USER CODE BEGIN StartMotorTask */
  // 1. 系统启动后先初始化马达
  Motor_Init_All();
  
  /* Infinite loop */
  for(;;)
  {
    // 这里写你的逻辑。比如：如果温度 > 30度，风扇(马达)全速转动
    
    if(temperature >= 30)
    {
        Motor_Set(90, 1); // 90%转速正转
    }
    else if(temperature < 28)
    {
        Motor_Stop();     // 温度降下来了就停止
    }
    
    osDelay(1000); // 没必要跑太快，1秒检查一次足够
  }
  /* USER CODE END StartMotorTask */
}

/* USER CODE END Application */

