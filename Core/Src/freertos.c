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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Pv */
// 1. 存放 ADC DMA 搬运来的原始数据 (对应 IN10, IN11, IN12)
uint16_t adc_raw_data[3] = {0};

// 2. 存放处理后的物理量 (给 GUI 显示用)

float shared_temp = 0.0f;      // 温度
float shared_ntu_volt = 0.0f;  // 浊度电压 (保留着，调试用)
float shared_ntu_val = 0.0f;   // 【新增】浊度百分比 (0-100)
uint16_t shared_level = 0;     // 液位
uint8_t shared_level_pct = 0;  // 水位百分比

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
osThreadId_t sensorTaskHandle;
void StartSensorTask(void *argument);
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
  sensorTaskHandle = osThreadNew(StartSensorTask, NULL, &defaultTask_attributes);

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
  // 1. 初始化 OLED (必须在任务里做，不能在中断里做)
  OLED_Init();
  OLED_Clear();
  
  // 2. 显示固定 UI 框架
  OLED_ShowString(0, 0, (u8*)"Water Monitor", 16);
  OLED_ShowString(0, 16, (u8*)"Temp:", 12);
  OLED_ShowString(0, 28, (u8*)"NTU :", 12);
  OLED_ShowString(0, 40, (u8*)"Lvl :", 12);
  
  OLED_Refresh();

  char disp_buf[32];

  /* Infinite loop */
  for(;;)
  {
    // --- 刷新温度值 ---
    // x=40 是为了避开 "Temp:" 标签
    // 后面加两个空格 "  " 是为了覆盖掉上次可能留下的长尾巴
    // sprintf(disp_buf, "%.1f C  ", shared_temp);
    // OLED_ShowString(40, 16, (u8*)disp_buf, 12);
    int temp_int = (int)shared_temp; 
    int temp_dec = (int)((shared_temp - temp_int) * 10); 
    if(temp_dec < 0) temp_dec = -temp_dec; // 防止负数bug
    sprintf(disp_buf, "%d.%d C  ", temp_int, temp_dec);
    OLED_ShowString(40, 16, (u8*)disp_buf, 12);

    // --- 刷新浊度电压 ---
    // sprintf(disp_buf, "%.2f V  ", shared_ntu_volt);
    // OLED_ShowString(40, 28, (u8*)disp_buf, 12);
    int ntu_int = (int)shared_ntu_val;
    int ntu_dec = (int)((shared_ntu_val- ntu_int) * 100); // 显示两位小数
    if(ntu_dec < 0) ntu_dec = -ntu_dec;
    sprintf(disp_buf, "%d.%02d %%  ", ntu_int, ntu_dec);
    OLED_ShowString(40, 28, (u8*)disp_buf, 12);

    // --- 刷新液位值 ---
    // 第一步：把原始值 (0~4095) 换算成百分比 (0.0~100.0)
    // 假设：你的传感器在满水时，Lvl_Raw 大概是 2500 (这个值你需要根据实测修改！)
    // 公式：当前值 / 满水值 * 100
    float level_val_float = (float)shared_level * 100.0f / 1100.0f;
    
    // 限制最大只能显示 100.00 %
    if(level_val_float > 100.0f) level_val_float = 100.0f;

    // 第二步：拆分整数和小数 (完全照搬你的格式逻辑)
    int lvl_int = (int)level_val_float;
    int lvl_dec = (int)((level_val_float - lvl_int) * 100); 
    if(lvl_dec < 0) lvl_dec = -lvl_dec; // 防止负数bug

    // 第三步：格式化并显示
    // 这里的 " %% " 是为了在 sprintf 里打印出一个百分号
    // x=40, y=40 是液位行的位置
    sprintf(disp_buf, "%d %%   ", shared_level_pct); 
    OLED_ShowString(40, 40, (u8*)disp_buf, 12);



    // --- 心跳点 (可选) ---
    static uint8_t tick = 0;
    if(tick) OLED_ShowString(110, 0, (u8*)".", 16);
    else     OLED_ShowString(110, 0, (u8*)" ", 16);
    tick = !tick;

    // 统一刷新显存到屏幕
    OLED_Refresh();

    // 屏幕刷新率不需要太高，500ms 一次足够人眼看了
    osDelay(500);
  }
}

void StartSensorTask(void *argument)
{
// 1. 初始化 DS18B20
  // 虽然 main.c 里没调，但在这里调最安全
  uint8_t dev_status = DS18B20_Init();

  // 2. 启动 ADC DMA (让硬件在后台自动读 IN10, IN11, IN12)
  // 注意：数组长度是 3
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw_data, 3);

  /* Infinite loop */
  for(;;)
  {
    // --- A. 读取 DS18B20 温度 ---
    shared_temp = DS18B20_Get_Temp();

    // --- B. 处理 浊度 (IN10 -> adc_raw_data[0]) ---
    // 浊度传感器输出的是电压，我们先显示电压，方便后续校准
    float voltage = adc_raw_data[0] * 3.3f / 4096.0f;
    shared_ntu_volt = voltage;
    // 逻辑：电压越高，水越干净(100)；电压越低，水越脏(0)
    float temp_val = voltage * 100.0f / 3.3f * 2;
    if(temp_val > 100.0f) temp_val = 100.0f; // 限制最大值
    
    // 如果你想要“数值越大越脏”，可以用 100 - temp_val
    shared_ntu_val = 100 - temp_val;

    // --- C. 处理 液位 (IN11 -> adc_raw_data[1]) ---
    shared_level = adc_raw_data[1];
    
    if(shared_level >= 1000)      shared_level_pct = 100;
    else if(shared_level >= 700) shared_level_pct = 90;
    else if(shared_level >= 450) shared_level_pct = 80;
    else if(shared_level >= 300) shared_level_pct = 70;
    else if(shared_level >= 230) shared_level_pct = 60;
    else if(shared_level >= 180) shared_level_pct = 50;
    else if(shared_level >= 140)  shared_level_pct = 40;
    else if(shared_level >= 125)  shared_level_pct = 30;
    else if(shared_level >= 100)  shared_level_pct = 20;
    else if(shared_level >= 80)  shared_level_pct = 10;
    else                          shared_level_pct = 0;

    // --- D. 串口打印 (调试用) ---
    // 这样你既可以在屏幕看，也可以在电脑串口助手看
    printf("Temp:%.2f C, NTU_V:%.2f V, Level:%d\r\n", 
           shared_temp, shared_ntu_volt, shared_level);

    // 每 1 秒采集一次，不要太快
    osDelay(1000);
  }
}
/* USER CODE END Application */

