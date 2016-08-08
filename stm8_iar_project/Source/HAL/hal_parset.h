#ifndef __hal__xor__h
#define __hal__xor__h

#include "stm8l15x.h"
#include "stdbool.h"
#include "hal_rf.h"

/*
 *******************************************************************************
 *                              macro processor
 *******************************************************************************
 */
#define CONFIG_MAX_DATA_LEN 16
#define CONFIG_WAIT_TIME    60  // 进入配置程序后等待上位机配置帧的超时时间值为60s

// 默认的网络参数
#define NET_LINK_ADDR       (uint16_t)1  // 默认的LinkAddr
#define NET_LINK_NET_ID     (uint16_t)1  // 默认的LinkNetId
#define NET_LINK_AREAL_ID   (uint32_t)3  // 默认的区域号
// 默认灯控参数
#define SEND_SW_PERIOD_MIN  (uint32_t)20   // 发送灯控制信号的默认周期为20秒
#define SEND_ON_TIME_MIN    (uint32_t)30   // 灯开启时间默认为30s
#define LIGHT_GROUP_ID      (uint8_t)1     // 默认的组ID为1
#define LIGHT_LIGHT_LEVAL   (uint8_t)80    // 默认的灯亮度为80%

// 参数修改的命令定义
#define CMD_CONFIG_NET_PAR      0x00   // 配置网络参数命令 (LinkAddr,LinkNetId,LinkArealId)
#define CMD_GET_NET_PAR         0x01   // 获取网络参数命令
#define CMD_CONFIG_LIGHT_PAR    0x02   // 配置灯参数命令 (灯开启时长，亮度以及运动探测所在的分组ID)
#define CMD_GET_LIGHT_PAR       0x03   // 获取灯参数命令
#define CMD_TERMINATE_PARLOOP   0x04   // 退出参数配置循环命令
#define CMD_GENERAL_ACK         0xff   // 通用应答指令 (数据包的基础解析,SUM XOR 或无对应命令时应答使用)

// 参数类型设置定义
#define PAR_SET_TYPE_LINK       0x00   // 配置类型为设备网络参数
#define PAR_SET_TYPE_LIGHT      0x01   // 配置类型为灯控参数

// 错误定义
#define ERR_CMD_ERR         0
#define ERR_OK              1
#define ERR_TIMEOUT         2
#define ERR_XOR_ERR         3
#define ERR_SUM_ERR         4
#define ERR_EEPROM_ERR      5

/*
 *******************************************************************************
 *                               user   type
 *******************************************************************************
 */
/* 串口收发数据包定义 */
typedef struct {
	uint8_t cmd;
	uint16_t length;
	uint8_t data[CONFIG_MAX_DATA_LEN];uint8_t xor;
	uint8_t sum;
	bool ifRecDone;   // 用于标志一个完成的数据包接收完成，每次处理完后置为false
} ParSettingDef_t;

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

/*
 *******************************************************************************
 *                               global   variable
 *******************************************************************************
 */

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
uint8_t HalXorMake(ParSettingDef_t parset);
uint8_t HalSumMake(ParSettingDef_t parset);
bool HalXorCheck(ParSettingDef_t parset);
bool HalSumCheck(ParSettingDef_t parset);

#endif

