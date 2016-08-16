#ifndef __HAL_LIGHT_H__
#define __HAL_LIGHT_H__
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "stm8l15x.h"

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalLightInit(void);

void HalLightOpen(void);

void HalLightClose(void);

void HalLightSetBrightness(uint8_t percent);

bool HalLightIsOpen(void);

uint8_t HalLightGetPWM(void);

#endif

