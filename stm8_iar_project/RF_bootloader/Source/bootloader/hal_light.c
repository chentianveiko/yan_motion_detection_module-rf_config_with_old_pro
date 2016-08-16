/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_light.h"


/*
 *******************************************************************************
 *                              LOCAL VARIABLES                                 
 *******************************************************************************
 */
static bool     HalLightOn;
static uint8_t  HalLigthPWM; /* 当前PWM输出百分比 */
   
/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalLightInit(void)
{
  /* Enable TIM1 clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);
  
  /* GPIOD configuration */
  GPIO_Init(GPIOD, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_Init(GPIOD, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Fast);
  GPIO_SetBits(GPIOD, GPIO_Pin_3); /* Close relay output */
  /*
  - TIM1CLK = 16 MHz
  - TIM1 counter clock = TIM1CLK / 320 = 16MHz/320 = 50000Hz
  */
  /* Time base configuration */
  TIM1_TimeBaseInit(320, TIM1_CounterMode_Up, 100, 0);
  
  TIM1_OC1Init(TIM1_OCMode_PWM1, TIM1_OutputState_Enable, TIM1_OutputNState_Disable,
               10, TIM1_OCPolarity_Low, TIM1_OCNPolarity_High, TIM1_OCIdleState_Set,
               TIM1_OCNIdleState_Set);
  TIM1_OC1PreloadConfig(DISABLE);
  
  TIM1_ARRPreloadConfig(ENABLE);
  
  /* Enable TIM1 outputs */
//  TIM1_CtrlPWMOutputs(ENABLE);
  //HalLightOpen();
//  HalLightClose();
//  HalLightSetBrightness(10);
  HalLigthPWM = (80);
  HalLightOpen();
  
}

static void HalLightDelay(void)
{
  volatile int a, b;
  
  
  for (a = 0; a < 300; a++)
  {
    for (b = 0; b < 200; b++)
    {
      __no_operation();
      __no_operation();
      __no_operation();
      __no_operation();
    }
  }
}

void HalLightOpen(void)
{
  TIM1_SetCompare1(5);
  TIM1_Cmd(ENABLE);
  TIM1_CtrlPWMOutputs(ENABLE);
  HalLightDelay();
  // GPIO_ResetBits(GPIOD, GPIO_Pin_3);
//  GPIO_SetBits(GPIOD, GPIO_Pin_3);
  HalLightDelay();
  for (int i = 5; i <= HalLigthPWM; i++)
  {
    HalLightDelay();
    TIM1_SetCompare1(i); 
  }
  HalLightOn = TRUE;
}


void HalLightClose(void)
{
  TIM1_SetCompare1(10);
  HalLightDelay();
//  GPIO_SetBits(GPIOD, GPIO_Pin_3);
  GPIO_ResetBits(GPIOD, GPIO_Pin_3);
  
  HalLightDelay();
  TIM1_Cmd(DISABLE);
  TIM1_CtrlPWMOutputs(DISABLE);
  GPIO_SetBits(GPIOD, GPIO_Pin_2);
  
  HalLightOn = FALSE;
  HalLigthPWM = 0;     /* 定时器关闭，无PWM输出 */
}

void HalLightSetBrightness(uint8_t percent)
{
  HalLigthPWM = percent;
  TIM1_SetCompare1(percent);
}

bool HalLightIsOpen(void)
{
  return HalLightOn;
}

uint8_t HalLightGetPWM(void)
{
  return HalLigthPWM;
}
