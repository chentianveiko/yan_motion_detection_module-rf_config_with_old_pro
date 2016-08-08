
#include "hal_key.h"

void HalKeyInit(void)
{
  GPIO_Init(KEY_1_PORT, KEY_1_PIN, GPIO_Mode_In_FL_IT);
  EXTI_SetPinSensitivity((EXTI_Pin_TypeDef)KEY_1_EXTI, KEY_1_EXTI_MODE);
}











