/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "bootloader.h"
#include "port.h"
#include "stm8l15x.h"

/*
 *******************************************************************************
 *                             MAIN - THE ENTER POINT                                  
 *******************************************************************************
 */
int main(void)
{
  BLStartup();
 
  BLStartApplication();
  
  return 0;
}


                
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 

  while (1)
  {
  }
}
#endif