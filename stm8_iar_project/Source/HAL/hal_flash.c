/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "hal_flash.h"

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
  uint8_t nCycle;

  FLASH_Unlock(FLASH_MemType_Data);

  if ( (data != NULL) && (length != 0) )
  {
    nCycle = (length / 4) + ((length % 4 > 0) ? 1 : 0);

    while (nCycle--)
    {
      FLASH_ProgramWord(addr, *(uint32_t *)data);
      while( (FLASH->IAPSR & (FLASH_IAPSR_EOP | FLASH_IAPSR_WR_PG_DIS)) == 0);

      addr += 4;
      data += 4;
    }

    reslut = length;
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


