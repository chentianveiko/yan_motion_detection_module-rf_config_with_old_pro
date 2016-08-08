#ifndef __HAL_FLASH_H__
#define __HAL_FLASH_H__
/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "hal_mcu.h"
//#include "etos.h"

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
void HalFlashInit(void);

uint16_t HalFlashWrite(uint32_t address, uint8_t *data, uint32_t length);

uint16_t HalFlashRead(uint32_t address, uint8_t *buffer, uint32_t size);

#endif

