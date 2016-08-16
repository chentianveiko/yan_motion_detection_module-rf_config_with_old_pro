/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_flash.h"
#include <string.h>

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalFlashInit(void)
{
  /* Define flash programming Time*/
  FLASH_SetProgrammingTime(FLASH_ProgramTime_Standard);
  
  FLASH_Unlock(FLASH_MemType_Program);
  /* Wait until Flash Program area unlocked flag is set*/
  while (FLASH_GetFlagStatus(FLASH_FLAG_PUL) == RESET)
  {
    ;
  }
}

uint16_t HalFlashWrite(uint32_t addr, uint8_t *data, uint32_t length)
{
  uint16_t reslut = 0;
  
  FLASH_Unlock(FLASH_MemType_Data);
  
  if ( (data != NULL) && (length != 0) )
  {
    reslut = length;
    
    while (length--)
    {
      FLASH_ProgramByte(addr, *data);
      addr++;
      data++;
    }
  }
  
  FLASH_Lock(FLASH_MemType_Data);
  
  return reslut;
}

uint16_t HalFlashRead(uint32_t addr, uint8_t *buffer, uint32_t size)
{
  uint16_t reslut = 0;
  
  
  if ( (buffer != NULL) && (size != 0) )
  {
    reslut = size;
    
    while (size--)
    {
      *buffer = FLASH_ReadByte(addr);
      addr++;
      buffer++;
    }
  }
  
  return reslut;
}


