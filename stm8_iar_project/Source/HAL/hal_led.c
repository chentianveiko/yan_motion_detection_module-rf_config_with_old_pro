
#include "hal_led.h"

/*
 *******************************************************************************
 @brief    初始化LED端口及引脚

 @params   none

 @return   none

 @author   Veiko

 @time     2016-6
 *******************************************************************************
 */
void Hal_led_init(void){
	GPIO_Init(HAL_LED_PORT,HAL_LED_RED_PIN | HAL_LED_BLUE_PIN, GPIO_Mode_Out_PP_High_Slow);
}
/*
 *******************************************************************************
 @brief    进入低功耗时LED引脚进行必要的配置

 @params   none

 @return   none

 @author   Veiko

 @time     2016-6
 *******************************************************************************
 */
void Hal_led_enterLowPowerMode(void){
	GPIO_Init(HAL_LED_PORT,HAL_LED_RED_PIN | HAL_LED_BLUE_PIN, GPIO_Mode_Out_PP_High_Slow);
	HAL_LED_RED_OFF();
	HAL_LED_BLUE_OFF();
}
