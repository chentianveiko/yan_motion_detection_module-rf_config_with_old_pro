#ifndef __HAL_MCU_H__
#define __HAL_MCU_H__
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "stm8l15x.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

/*
 *******************************************************************************
 *                                  MACROS                                  
 *******************************************************************************
 */
#define HAL_MCU_REBOOT()        asm("jpf $8000")

#define HTONS(n)  (n)//(uint16_t)((((n) & 0xFF) << 8) | (((n) & 0xFF00) >> 8))
#define HTONL(n)  (n)//(uint32)((((n) & 0xFF) << 24) | (((n) & 0xFF00) << 8) | (((n) & 0xFF0000UL) >> 8) | (((n) & 0xFF000000UL) >> 24))
#define NTOHS(n)  HTONS(n)
#define NTOHL(n)  HTONL(n)   

#define PRAGMA(x) _Pragma(#x)
#define HAL_MCU_DATA_ALIGN(n) //PRAGMA(data_alignment=n)

#define HAL_MCU_ENTER_SLEEP() HalMcuEnterSleep()
#define HAL_MCU_EXIT_SLEEP()  HalMcuExitSleep()

// stm8唯一序列号存储类型定义
typedef uint8_t hw_mcu_unique_id_t[12];

#define HW_MCU_READ_DEVICE_ID(id)  if (id != NULL) memcpy(id, (uint8_t *)0x4926, sizeof(hw_mcu_unique_id_t))

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define HAL_MCU_FLASH_BEGIN_ADDR        0xA800
#define HAL_MCU_FLASH_END_ADDR          0x17FFF
// EEPROM块0起止地址   
#define HAL_EEPROM_B0_BEGIN_ADDR           0x001000
#define HAL_EEPROM_B0_END_ADDR             0x00103F

// FLash初始化标志存储说明
#define FLASH_INIT_FLAG_ADDR          (HAL_EEPROM_B0_BEGIN_ADDR+0)
#define FLASH_INIT_FLAG_SIZE          4
#define FLASH_INIT_FLAG_ENDADDR       (HAL_EEPROM_B0_BEGIN_ADDR+3)

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
/* Board enter sleep mode */
void HalMcuEnterSleep(void);

/* Board exit sleep  mode */
void HalMcuExitSleep(void);

void HalMcuEnterLowPowerRunMode(void);
void HalMcuExitLowPowerRunMode(void);
void HalMcuEnterLowPowerWaitMode(void);
void HalMcuExitLowPowerWaitMode(void);

void _hw_board_random_init(void);
int32_t hw_board_random_get(void);

#endif

