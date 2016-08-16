#ifndef __PORT_H__
#define __PORT_H__
/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "stm8l15x.h"
#include "bootloader.h"
#include <stdlib.h>
#include "link.h"

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#ifndef NULL
#define NULL (void *)0
#endif

#ifndef false
#define false FALSE
#endif

#ifndef true
#define true  TRUE
#endif

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#define APP_BEGIN_ADDRESS   0xA800
#define APP_MAX_SIZE        (1024ul * 22ul)

/*
 *******************************************************************************
 *                                  MACROS
 *******************************************************************************
 */
#define port_malloc(n) malloc(n)
#define port_free(p)   free(p)

#define BL_HTONS(n)    (n)//(((n) & 0xFF) << 8) | (((n) & 0xFF00) >> 8)
#define BL_NTOHS(n)    BL_HTONS(n)
#define BL_HTONL(n)    (n)//(((n) & 0xFF) << 24) |(((n) & 0xFF00) << 8) | (((n) & 0xFF0000UL) >> 8) | (((n) & 0xFF000000UL) >> 24);
#define BL_NTOHL(n)    BL_HTONL(n)


#define PRAGMA(x) _Pragma(#x)
#define HAL_MCU_DATA_ALIGN(n) //PRAGMA(data_alignment=n)

#define BL_POLL()   LinkInternalProcess(); BLWdgReload();


/* 设备网络参数定义 */
typedef struct {
	uint16_t LinkAddr;     // 设备地址
	uint16_t LinkNetId;    // 设备网络地址
	uint32_t LinkArealId;  // 设备区域号
	bool enableRoute;      // 是否具备路由功能
} LinkParDef;

/* 灯控制参数定义 */
typedef struct {
	uint8_t groupId;    // 组ID
	uint8_t Leval;      // 亮度
	uint32_t ON_seconds; // 开启时长/s
	uint32_t send_peroid; // 发送开关信号的最小周期
} LightParDef;

// 设备参数结构体定义
typedef struct {
	uint32_t init_flag;
	LinkParDef net_config;
	LightParDef ctr_config;
	uint8_t rf_channel;      // 设备使用的RF信道
	uint8_t boot_rf_channel;// 用于升级的信道
}imd_config_t;


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
BLResult_t BLPortInit(void);

/*
 *******************************************************************************
  @brief   启动1秒定时器

  @params  timerCallback - 定时回调函数

  @return   无
 *******************************************************************************
 */
void BLStartTimer( void (* timerCallback)(void) );

/*
 *******************************************************************************
  @brief    停止1秒定时器

  @params   无

  @return   无
 *******************************************************************************
 */
void BLStopTimer();

/*
 *******************************************************************************
  @brief    Flash擦除

  @params   addr   - 擦除起始地址
            length - 擦除长度

  @return   见 BLResult_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashErase(uint32_t addr, uint32_t length);

/*
 *******************************************************************************
  @brief    Flash写数据操作

  @params   addr   - 写数据起始地址
            data   - 待写入的数据
            length - 待写入的数据长度

  @return   见 iap_result_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashWrite(uint32_t addr, uint8_t *data, uint32_t length);

/*
 *******************************************************************************
  @brief    Flash读数据操作

  @params   addr   - 读取数据的起始地址
            data   - 数据存入的Buffer
            length - 读取的数据长度

  @return   见 BLResult_t 定义
 *******************************************************************************
 */
BLResult_t BLFlashRead(uint32_t addr, uint8_t *buffer, uint32_t length);

/*
 *******************************************************************************
  @brief    底层发送数据包

  @params   data  - 发送的报文
	    length- 报文长度

  @return   实际发送的字节数，改善失败返回0
 *******************************************************************************
 */
uint16_t BLLowDataSend(uint8_t *data, uint16_t length);

/*
 *******************************************************************************
  @brief    启动应用程序

  @params   无

  @return   无
 *******************************************************************************
 */
void BLStartApplication(void);

/*
 *******************************************************************************
  @brief    重启MCU

  @params   无

  @return   无
 *******************************************************************************
 */
void BLSystemReboot(void);

/*
 *******************************************************************************
  @brief    复位看门狗

  @params   无

  @return   无
 *******************************************************************************
 */
void BLWdgReload(void);
#endif



