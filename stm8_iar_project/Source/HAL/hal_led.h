#ifndef hal_led__h
#define hal_led__h

#include "stm8l15x.h"
#include "stdbool.h"

#define HAL_LED_PORT        GPIOD
#define HAL_LED_RED_PIN     GPIO_Pin_0
#define HAL_LED_BLUE_PIN    GPIO_Pin_3

#define HAL_LED_RED_OFF()     GPIO_SetBits(HAL_LED_PORT,HAL_LED_RED_PIN)
#define HAL_LED_RED_ON()    GPIO_ResetBits(HAL_LED_PORT,HAL_LED_RED_PIN)
#define HAL_LED_RED_Toggle() GPIO_ToggleBits(HAL_LED_PORT,HAL_LED_RED_PIN)

#define HAL_LED_BLUE_OFF()     GPIO_SetBits(HAL_LED_PORT,HAL_LED_BLUE_PIN)
#define HAL_LED_BLUE_ON()    GPIO_ResetBits(HAL_LED_PORT,HAL_LED_BLUE_PIN)
#define HAL_LED_BLUE_Toggle() GPIO_ToggleBits(HAL_LED_PORT,HAL_LED_BLUE_PIN)

void Hal_led_init(void);
void Hal_led_enterLowPowerMode(void);

#endif

