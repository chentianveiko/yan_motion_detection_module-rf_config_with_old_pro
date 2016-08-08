#ifndef __HAL_IR_SENSOR_H__
#define __HAL_IR_SENSOR_H__
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_mcu.h"
#include "stdbool.h"
#include "stdbool.h"

extern bool ifIrWakeuped;

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalIRSensorInit(void);
bool HalIR_GetOutPutStu(void);

#endif

