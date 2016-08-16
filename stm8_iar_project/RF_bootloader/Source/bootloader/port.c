/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "port.h"
#include "frame.h"
#include <string.h>
#include "hal_rf.h"
#include "hal_flash.h"
#include "hal_light.h"
#include "virtual_spi.h"


/*
 *******************************************************************************
 *                              EXTERNAL FUNCTIONS
 *******************************************************************************
 */
extern void __iar_program_start(void);

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#define BOOTLOADER_DATA_TYPE    0x55
#define CONFIG_IN_FLASH_ADDR    0x1000          /* At EEPROM */

// 设备默认参数定义
#define FLASH_INIT_FLAG_VALUE         0x3A5A9AAA        // 设备参数初始化标志
#define IMD_LINK_ADDR_DEFAULT         1                 // 默认设备地址
#define IMD_LINK_NET_ADDR_DEFAULT     1                 // 默认网络地址
#define IMD_LINK_AREAL_ID_DEFAULT     4                 // 默认区域地址
#define IMD_LINK_GROUP_ID_DEFAULT     1                 // 默认组ID
#define IMD_RF_CHANNEL_DEFAULT        HAL_RF_CHANNEL_0  // 默认RF使用的信道
#define IMD_BOOT_RF_CHANNEL_DEFAULT   HAL_RF_CHANNEL_0  // 默认Boot使用的RF信道
#define IMD_LIGHT_LV_DEFAULT          100               // 默认的运动探测调光亮度
#define IMD_LIGHT_SEND_PERIOD_DEFAULT 20                // 默认的运动探测发送最小周期30秒(解发开灯动作的最小间隔)
#define IMD_LIGHT_ON_SECOND_DEFAULT   3               // 默认的开灯点亮时间为120秒

/*
 *******************************************************************************
 *                                  TYPEDEFS
 *******************************************************************************
 */
typedef struct
{
  uint16_t b;
  uint16_t v;
}BLSTM8Vector_t;

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#define USART_DMA_CHANNEL_RX   DMA1_Channel2
#define USART_DMA_CHANNEL_TX   DMA1_Channel1
#define USART_DMA_FLAG_TCRX    DMA1_FLAG_TC2
#define USART_DMA_FLAG_TCTX    DMA1_FLAG_TC1
#define USART_DR_ADDRESS       (uint16_t)0x5231  /* USART1 Data register Address */
#define USART_DMA_BUFFER_SIZE  128

/*
 *******************************************************************************
 *                              LOCAL VARIABLES
 *******************************************************************************
 */
static const BLSTM8Vector_t BLSTM8VectorList[] @ ".intvec" =
{
  {0x8200, (uint16_t)__iar_program_start},/* Reset vector, jump to __iar_program_start */
  {0x8200, APP_BEGIN_ADDRESS + 1  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 2  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 3  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 4  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 5  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 6  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 7  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 8  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 9  * 4},
  {0x8200, APP_BEGIN_ADDRESS + 10 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 11 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 12 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 13 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 14 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 15 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 16 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 17 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 18 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 19 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 20 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 21 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 22 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 23 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 24 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 25 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 26 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 27 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 28 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 29 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 30 * 4},
  {0x8200, APP_BEGIN_ADDRESS + 31 * 4},
};

void (* BLTimerCallbackFn)(void);

/* 需读取设备在EEPROM中的配置来设置LINK及RF参数 */
imd_config_t DeviceConfig;

LinkAddr_t SourceAddress;

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
__interrupt void BLTimerInterrupt(void);
extern __interrupt void EXTI3_IRQHandler();

static void BLCPUInit(void);
static void BLWDGInit(void);
static void BLFlashInit(void);
static void BLTimerInit(void);
static void ReadDeviceConfig(void);
static void BLLinkReceiveCallback(struct LinkMessage *message);

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
/*
 *******************************************************************************
  @brief    初始化Flash设备

  @params   无

  @return   见 BLResult_t 定义
 *******************************************************************************
 */
BLResult_t BLPortInit(void)
{
  BLCPUInit();
  //HalLightInit();
  BLFlashInit();
  spi_init();
  HalRFInit();
  disableInterrupts();
  /* 等待一下再修改向量，以防在使用ST Visual Programmer编程校验时
     失败(ST Visual Programmer在校验程序时MCU已经在执行程序了) */
  for (uint32_t i = 0; i < 110000; i++) asm("nop");
  /* 开启中断前，重新映射RF引脚中断向量, TIMER 向量 */
  BLSTM8Vector_t *p = (BLSTM8Vector_t *)&BLSTM8VectorList[13];
  p->v = (uint16_t)EXTI3_IRQHandler;
  p = (BLSTM8Vector_t *)&BLSTM8VectorList[27];
  p->v = (uint16_t)BLTimerInterrupt;

  ReadDeviceConfig();

  LinkInit();
  LinkSetAreaId(DeviceConfig.net_config.LinkArealId);
  LinkSetNetId(DeviceConfig.net_config.LinkNetId);
  LinkSetAddress(DeviceConfig.net_config.LinkAddr);
  LinkSetReceiveCallback(BLLinkReceiveCallback);

  //BLWDGInit();

  HalRFSetChannel(HalRF1, (HalRFChannel_t)DeviceConfig.boot_rf_channel);
  BLTimerInit();
  enableInterrupts();

  return NO_ERROR;
}

/*
 *******************************************************************************
  @brief   启动1秒定时器

  @params  timerCallback - 定时回调函数

  @return   无
 *******************************************************************************
 */
void BLStartTimer( void (* timerCallback)(void) )
{
  BLTimerCallbackFn = timerCallback;

  /* Enable TIM4 */
  TIM4_Cmd(ENABLE);
}

/*
 *******************************************************************************
  @brief    停止1秒定时器

  @params   无

  @return   无
 *******************************************************************************
 */
void BLStopTimer()
{
  /* Disable TIM4 */
  TIM4_Cmd(DISABLE);
  TIM4_ITConfig(TIM4_IT_Update, DISABLE);
}

/*
 *******************************************************************************
  @brief    Flash擦除

  @params   addr   - 擦除起始地址
            length - 擦除长度

  @return   见 BLResult_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashErase(uint32_t addr, uint32_t length)
{
  return NO_ERROR;
}

/*
 *******************************************************************************
  @brief    Flash写数据操作

  @params   addr   - 写数据起始地址
            data   - 待写入的数据
            length - 待写入的数据长度

  @return   见 iap_result_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashWrite(uint32_t addr, uint8_t *data, uint32_t length)
{
  uint8_t nCycle;

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

    return NO_ERROR;
  }

  return INVALID_PARAMETER;
}

/*
 *******************************************************************************
  @brief    Flash读数据操作

  @params   addr   - 读取数据的起始地址
            data   - 数据存入的Buffer
            length - 读取的数据长度

  @return   见 BLResult_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashRead(uint32_t addr, uint8_t *buffer, uint32_t length)
{
  if ( (buffer != NULL) && (length != 0) )
  {
    while (length--)
    {
      *buffer = (*(PointerAttr uint8_t *) (MemoryAddressCast)addr);

      addr++;
      buffer++;
    }

    return NO_ERROR;
  }

  return INVALID_PARAMETER;
}

extern int HalRFTxFlag;
/*
 *******************************************************************************
  @brief    底层发送数据包

  @params   data  - 发送的报文
	    length- 报文长度

  @return   实际发送的字节数，改善失败返回0
 *******************************************************************************
 */
uint16_t BLLowDataSend(uint8_t *data, uint16_t length)
{
  if ( (data != NULL) && (length != 0) )
  {
    while (!HalRFTxFlag);
    HalRFTxFlag = FALSE;
    return LinkSend(0, BOOTLOADER_DATA_TYPE, data, length);
  }

  return 0;
}

/*
 *******************************************************************************
  @brief    启动应用程序

  @params   无

  @return   无
 *******************************************************************************
 */
void BLStartApplication(void)
{
//  for (int i = 0; i < 15000; i++) asm("nop");
  while (!HalRFTxFlag);

//  TIM4_DeInit();
//  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);

  disableInterrupts();
  /* 关闭中断后重新映射RF引脚中断微量, TIM4中断向量 */
  BLSTM8Vector_t *p = (BLSTM8Vector_t *)&BLSTM8VectorList[13];
  p->v = (uint16_t)APP_BEGIN_ADDRESS + 13 * 4;

  p = (BLSTM8Vector_t *)&BLSTM8VectorList[27];
  p->v = (uint16_t)APP_BEGIN_ADDRESS + 27 * 4;

  FLASH_Lock(FLASH_MemType_Program);

  /* Jump to App startup address - APP_BEGIN_ADDRESS */
  asm("jpf $A800");
}

/*
 *******************************************************************************
  @brief    重启MCU

  @params   无

  @return   无
 *******************************************************************************
 */
void BLSystemReboot(void)
{
  for (int i = 0; i < 3000; i++) asm("nop"); /* 等待应答完成 */
  asm("jpf $8000");
}

/*
 *******************************************************************************
  @brief    复位看门狗

  @params   无

  @return   无
 *******************************************************************************
 */
void BLWdgReload(void)
{
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
static void BLCPUInit(void)
{
//  CLK_HSICmd(ENABLE);
//  while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET);
//
//  /* system clock prescaler: 1*/
//  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
//  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI);
//
//  /* Select HSI as system clock source */
//  CLK_SYSCLKSourceSwitchCmd(ENABLE);
//  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);

  /* Select HSE as system clock source */
  CLK_SYSCLKSourceSwitchCmd(ENABLE);
  CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSE);

  /*High speed external clock prescaler: 1*/
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

  while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE)
  {}
}

static void BLWDGInit()
{
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();

  /* IWDG timeout equal to 214 ms (the timeout may varies due to LSI frequency
  dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG configuration: IWDG is clocked by LSI = 38KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* (254 + 1) * 256 / 38 000 ~= 1700 ms */
  IWDG_SetReload((uint8_t)254);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();
}

static void BLTimerInit(void)
{
   /* Enable TIM4 Clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

  /* Configure TIM4 to generate an update event each 0.5 s */
  TIM4_TimeBaseInit(TIM4_Prescaler_32768, 244);

  /* Clear the Flag */
  TIM4_ClearFlag(TIM4_FLAG_Update);

  TIM4_ITConfig(TIM4_IT_Update, ENABLE);
}

static void BLFlashInit(void)
{
  HalFlashInit();
}

/*
 *******************************************************************************
  @brief    配置信息初始化，当flash中没有配置时初始化为默认配置，否则读取flash中的
            配置信息

  @params   无

  @return   无
 *******************************************************************************
 */
static void ReadDeviceConfig(void)
{
	HalFlashRead(CONFIG_IN_FLASH_ADDR, (uint8_t *) (&DeviceConfig), sizeof(imd_config_t));

	if (DeviceConfig.init_flag != FLASH_INIT_FLAG_VALUE) {
		DeviceConfig.init_flag = FLASH_INIT_FLAG_VALUE;

		DeviceConfig.rf_channel = IMD_RF_CHANNEL_DEFAULT;

		DeviceConfig.net_config.LinkAddr = IMD_LINK_ADDR_DEFAULT;
		DeviceConfig.net_config.LinkArealId = IMD_LINK_AREAL_ID_DEFAULT;
		DeviceConfig.net_config.LinkNetId = IMD_LINK_NET_ADDR_DEFAULT;
		DeviceConfig.net_config.enableRoute = FALSE;

		DeviceConfig.ctr_config.Leval = IMD_LIGHT_LV_DEFAULT;
		DeviceConfig.ctr_config.groupId = IMD_LINK_GROUP_ID_DEFAULT;
		DeviceConfig.ctr_config.ON_seconds = IMD_LIGHT_ON_SECOND_DEFAULT;
		DeviceConfig.ctr_config.send_peroid = IMD_LIGHT_SEND_PERIOD_DEFAULT;
		DeviceConfig.boot_rf_channel = IMD_BOOT_RF_CHANNEL_DEFAULT;

		HalFlashWrite(CONFIG_IN_FLASH_ADDR, (uint8_t *) (&DeviceConfig), sizeof(imd_config_t));
	}
}

static void BLLinkReceiveCallback(struct LinkMessage *message)
{
  if (BOOTLOADER_DATA_TYPE == message->type)
  {
    SourceAddress = message->srcAddr;
    BLDataHandler(message->data, message->length);
  }
}

__interrupt void BLTimerInterrupt(void)
{
  static int t = 0;


  t++;
  TIM4_ClearITPendingBit(TIM4_IT_Update);
  if ( (0 == (t % 2)) && (BLTimerCallbackFn != NULL) ) BLTimerCallbackFn();
}