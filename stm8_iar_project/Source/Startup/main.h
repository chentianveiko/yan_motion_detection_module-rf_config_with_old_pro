#ifndef __main__h
#define __main__h

#include "stm8l15x.h"
#include "link.h"
#include "stm8l15x.h"
#include "hal_rf.h"
#include "hal_ir_sensor.h"
#include "virtual_spi.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "hal_flash.h"
#include "hal_key.h"
#include "hal_rtc.h"
#include "hal_flash_app.h"
#include "hal_mcu.h"

/* 是否开启串口调试输出 1表示开启 非1表示关闭 */
#define UsartDebug 0

// 程序版本
#define APP_VERSION      0
#define APP_SUBVERSION   0
#define APP_REVISION     1

// 时钟配置时时钟类型定义
#define CLK_TYPE_LSE  0
#define CLK_TYPE_HSI  1
#define CLK_TYPE_LSI  3
#define CLK_TYPE_HSE  4

// 开机后的参数配置时间
#define CONFIG_TIME_RESTART           60                // 重启后的配置时间，单位秒

// 设备默认参数定义
#define FLASH_INIT_FLAG_VALUE         0x3A5A9AAA        // 设备参数初始化标志
#define IMD_LINK_ADDR_DEFAULT         0                 // 默认设备地址
#define IMD_LINK_NET_ADDR_DEFAULT     1                 // 默认网络地址
#define IMD_LINK_AREAL_ID_DEFAULT     4                 // 默认区域地址
#define IMD_LINK_GROUP_ID_DEFAULT     1                 // 默认组ID
#define IMD_RF_CHANNEL_DEFAULT        HAL_RF_CHANNEL_0  // 默认RF使用的信道
#define IMD_LIGHT_LV_DEFAULT          100               // 默认的运动探测调光亮度
#define IMD_LIGHT_SEND_PERIOD_DEFAULT 5                // 默认的运动探测发送最小周期30秒(解发开灯动作的最小间隔)
#define IMD_LIGHT_ON_SECOND_DEFAULT   2               // 默认的开灯点亮时间为120秒

// 设备类型定义
#define DEV_TP_LAMP_CONTROLLER  0x00    // 设备类型--灯控制器
#define DEV_TP_IM_DETECTER      0x01    // 设备类型--红外运动探测器
#define DEV_TP_WL_SWITCHER      0x02    // 设备类型--无线开关模块

// 消息类型定度
#define MESSAGE_TP_SET_NP_REQ             0x20    // 设置网络参数命令类型(运动探测和无线开关有效)
#define MESSAGE_TP_GET_NP_REQ             0x21    // 获取网络参数命令类型(运动探测和无线开关有效)
#define MESSAGE_TP_SET_MTP_REQ            0x22    // 设置运动探测控制参数命令类型(仅运动探测模块有效)
#define MESSAGE_TP_GET_MTP_REQ            0x23    // 获取运动探测控制参数命令类型(仅运动探测模块有效)
#define LIGHT_CONTROL_PACKET              0x01
#define LIGHT_GROUP_CONTROL_PACKET        0x02
#define LIGHT_IR_CONTROL                  0x03
#define LINK_PARAMETER_GET                0xA0
#define LINK_PARAMETER_GET_RES            0xA1
#define LINK_PARAMETER_SET                0xA2
#define LINK_PARAMETER_SET_RES            0xA3
#define LINK_DEVICE_SCAN                  0xA4  // 设备扫描
#define LINK_DEVICE_SCAN_RESPONSE         0xA5  // 设备扫描应答
#define LINK_DEVICE_MAC_MATCH_INDICATE    0xA6
#define LINK_DEVICE_ROUTER_CONTORL        0xA7
#define LINK_DEVICE_ROUTER_CONTORL_RES    0xA8
#define LINK_DEVICE_ROUTER_GET            0xA9
#define LINK_DEVICE_ROUTER_GET_RES        0xAA
#define LINK_SET_ROUTE_DELAY              0xAB
#define LINK_SET_ROUTE_DELAY_RES          0xAC
#define LINK_GET_ROUTE_DELAY              0xAD
#define LINK_GET_ROUTE_DELAY_RES          0xAE
#define LINK_TEST                         0xAF
#define LINK_TEST_RESPONSE                0xB0
#define LINK_SET_CHANNEL                  0xB1   // 设置信道
#define LINK_SET_CHANNEL_RES              0xB2   // 设置信道的应答
#define LINK_GET_PWM                      0xB3
#define LINK_GET_PWM_RES                  0xB4
#define LINK_GET_TEMPERATURE              0xB5
#define LINK_GET_TEMPERATURE_RES          0xB6
#define LINK_GET_POWER_STATE              0xB7
#define LINK_GET_POWER_STATE_RES          0xB8
#define LINK_SET_SRPAN_PARAMETER          0xB9
#define LINK_SET_SRPAN_PARAMETER_RES      0xBA
#define LINK_GET_SRPAN_PARAMETER          0xBB
#define LINK_GET_SRPAN_PARAMETER_RES      0xBC

#define LINK_ENTER_BOOTLOADER             0x53
#define LINK_ENTER_BOOTLOADER_RES         0x54

/* SRPAN PACKET DEFINES */
#define SRPAN_UPLOAD_DEVICE_STATE        0x70
#define SRPAN_SET_GROUP_ID               0x71
#define SRPAN_SET_GROUP_ID_ACK           0x72

// 版本号结构体
typedef struct {
	uint8_t major;
	uint8_t minor;
	uint8_t revise;
} AppVersion_t;

HAL_MCU_DATA_ALIGN(1)
// 用于回复扫描命令消息的结构体定义
typedef struct {
	uint8_t deviceType;              // 设备类型
	hw_mcu_unique_id_t uniqueId;
	LinkAreaId_t areaId;
	LinkNetId_t netId;
	LinkAddr_t address;
	bool enableRoute;
	AppVersion_t version;
	uint8_t channel;
}ScanResponse_t;

HAL_MCU_DATA_ALIGN(1)
// 设备参数结构体定义
typedef struct {
	uint32_t init_flag;
	LinkParDef net_config;
	LightParDef ctr_config;
	HalRFChannel_t rf_channel;      // 设备使用的RF信道
}imd_config_t;

extern LinkAddr_t SourceAddress;
extern imd_config_t device_config;

void SystemClockInit(uint8_t Clk_Sel);
void rf_data_receive_handler(struct LinkMessage *message);
void ConfigLoop(void);

void imd_config_load(void);
void imd_config_restore(void);

void imd_rf_respond_scan_message(void);
void imd_rf_setnetPar_deal(struct LinkMessage *message);
void imd_rf_getnetPar_deal(struct LinkMessage *message);
void imd_rf_setlightPar_deal(struct LinkMessage *message);
void imd_rf_getlightPar_deal(struct LinkMessage *message);
void imd_rf_setrfChannel(struct LinkMessage *message);

#endif

