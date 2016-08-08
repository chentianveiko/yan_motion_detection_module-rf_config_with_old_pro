
#ifndef hal_key____H
#define hal_key____H

#include "stm8l15x.h"

#define KEY_1_PORT         GPIOD
#define KEY_1_PIN          GPIO_Pin_4
#define KEY_1_EXTI         EXTI_Pin_4
#define KEY_1_EXTI_MODE    EXTI_Trigger_Falling

void HalKeyInit(void);








#endif


