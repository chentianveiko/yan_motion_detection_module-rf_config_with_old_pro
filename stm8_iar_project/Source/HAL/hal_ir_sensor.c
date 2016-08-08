/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_ir_sensor.h"

/* 
 * 当唤醒后时间间隔小于设定值时，将该变量置为true表示在rtc的闹钟唤醒期间有过红外触发
  * 一旦RTC闹被唤醒，发现该值为true，就会通过无线发送一次灯控信号
*/
bool ifIrWakeuped = false;  

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalIRSensorInit(void)
{
  //GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Slow);  // 红外模块的使能(暂时被硬件取代)
  GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Slow);     // 不使用的情况下使用低功耗模式
  
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_In_FL_IT);
  EXTI_SetPinSensitivity(EXTI_Pin_0, EXTI_Trigger_Rising); 
}

bool HalIR_GetOutPutStu(void)
{
  if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == SET)
  {
    return true;
  }
  return false;
}