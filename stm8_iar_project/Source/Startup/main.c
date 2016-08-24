/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "main.h"
#include "string.h"
#include "stdio.h"

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */

/*
 *******************************************************************************
 *                             PUBLIC VARIABLES
 *******************************************************************************
 */
extern int Hal_RfTxComplete;

LinkAddr_t SourceAddress;
imd_config_t device_config;
bool ifEndConfigFlag = false;

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
static void rf_data_receive_handler(struct LinkMessage *message);
/*
 *******************************************************************************
 * 函数功能：初始化系统时钟
 * 函数参数：Clk_Sel -- CLK_TYPE_HSI/CLK_TYPE_LSE
 *******************************************************************************
 */
void SystemClockInit(uint8_t Clk_Sel) {
	switch (Clk_Sel) {
	case CLK_TYPE_LSE:  // 32.768KHz
		if (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_LSE) {
			CLK_LSEConfig (CLK_LSE_ON);
			while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET)
				;   // 等待LSE稳定

			/* Select LSE as system clock source */
			CLK_SYSCLKSourceSwitchCmd (ENABLE);
			CLK_SYSCLKSourceConfig (CLK_SYSCLKSource_LSE);

			/* system clock prescaler: 1*/
			CLK_SYSCLKDivConfig (CLK_SYSCLKDiv_1);
			while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_LSE)
				;

			CLK_HSICmd (DISABLE);
			CLK_SYSCLKSourceSwitchCmd(DISABLE);
		}

		break;
	case CLK_TYPE_LSI:
		if (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_LSI) {
			CLK_LSICmd (ENABLE);
			while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET)
				;

			CLK_SYSCLKSourceSwitchCmd(ENABLE);
			CLK_SYSCLKSourceConfig (CLK_SYSCLKSource_LSI);

			/* system clock prescaler: 1*/
			CLK_SYSCLKDivConfig (CLK_SYSCLKDiv_1);
			while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_LSI)
				;

			CLK_HSICmd (DISABLE);
			CLK_SYSCLKSourceSwitchCmd(DISABLE);
		}
		break;
	case CLK_TYPE_HSI:
		if (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI) {
			CLK_HSICmd (ENABLE);
			while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == RESET)
				;

			/* Select HSI as system clock source */
			CLK_SYSCLKSourceSwitchCmd(ENABLE);
			CLK_SYSCLKSourceConfig (CLK_SYSCLKSource_HSI);

			/* system clock prescaler: 1*/
			CLK_SYSCLKDivConfig (CLK_SYSCLKDiv_1);
			while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
				;

			CLK_SYSCLKSourceSwitchCmd (DISABLE);
		}

		break;

	case CLK_TYPE_HSE:
		if (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE) {
			CLK_HSEConfig (CLK_HSE_ON);
			while (CLK_GetFlagStatus(CLK_FLAG_HSERDY) == RESET)
				;

			/* Select HSE as system clock source */
			CLK_SYSCLKSourceSwitchCmd (ENABLE);
			CLK_SYSCLKSourceConfig (CLK_SYSCLKSource_HSE);

			/* system clock prescaler: 1*/
			CLK_SYSCLKDivConfig (CLK_SYSCLKDiv_1);
			while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSE)
				;

			CLK_SYSCLKSourceSwitchCmd (DISABLE);
		}
		break;
	default:
		break;
	}
}

/*
 *******************************************************************************
 @brief    组播控制

 @params   group   - 被控灯所在的组号
 level   - 灯的亮度值0~100, 小于10时为关灯
 seconds - 持续多少时间后恢复先前状态

 @return   无
 *******************************************************************************
 */
static void IrLightControl(uint8_t group, uint8_t level, uint32_t seconds) {
	HAL_MCU_DATA_ALIGN(1)
	struct req {
		uint8_t group_id;
		uint8_t level;
		uint32_t seconds;
	};

	struct req *preq;
	uint8_t buffer[sizeof(struct req)];

	preq = (struct req *) buffer;
	preq->group_id = group;
	preq->level = level;
	preq->seconds = HTONL(seconds);
	LinkSend(0xFFFF, LIGHT_IR_CONTROL, buffer, sizeof(buffer));
}
/*
 *******************************************************************************
 @brief   主循环，在开机或重启后，经过配置循环后进入主循环，主循环根据配置的信息来检测
 红外探测事件和闹钟唤醒事件，并根据配置信息来进行灯控制器的组播控制，一次任务完成后，系
 统会进入低功耗状态

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void MainLooper(void) {
	bool ifNeedSend = false;

	Hal_RfTxComplete = 0;
	while (1) {
        HalTIM4_Config();

		GetRtcValue();  // 获取当前RTC时间及日期参数

		if (ifIrWakeuped == true) {

			// 当两次唤醒时间间隔超过设定值
			//if (HalTimeCompare(RTC_TimeStr, RTC_DateStr) > device_config.ctr_config.send_peroid) {
			ifIrWakeuped = false;
			ifNeedSend = true; // 准备发送灯控信号
			//}
		}

		// 判断是否是闹钟唤醒的
		if (true == ifRtcAlarmWakeup) {
			ifRtcAlarmWakeup = false;

			if ((HalIR_GetOutPutStu() == true) || (ifIrWakeuped == true)) {
				ifIrWakeuped = false;
				// 准备发送一个灯控制信号
				ifNeedSend = true;
			} else {
				;
			}

			HalResetRtcAlarm();
		}

		// 发送灯控信号
		if (ifNeedSend == true) {
			ifNeedSend = false;
			// 保存本次发送的唤醒时间,以用作下次唤醒后判断
			memcpy(&RTC_DateStr_bck, &RTC_DateStr, sizeof(RTC_DateTypeDef));
			memcpy(&RTC_TimeStr_bck, &RTC_TimeStr, sizeof(RTC_TimeTypeDef));

			// 发送开灯信号--连续发送两次
			//for (uint8_t i = 0; i < 2; i++) {
              Hal_RfTxComplete = 0;

              IrLightControl(device_config.ctr_config.groupId, device_config.ctr_config.Leval, (device_config.ctr_config.ON_seconds) * 1000); // 发送分组灯控制信号，其中10000是灯开的延时，超过这个时间后，回复到灯的前一个状态
                // 等待发送完成
				HalRestartRunTimer(GenRunTimerID, 4, RT_TP_SECOND);
				while (0 == Hal_RfTxComplete) {
					if (HalgetRunTimerCnt(GenRunTimerID) == 0) {
						break;
					}
				}
                //HAL_LED_RED_ON();
                //HalRunTimerDelayms(100);
                //HAL_LED_RED_OFF();
				Hal_RfTxComplete = 0;
                //HalRunTimerDelayms(1000);
			//}

			// 发送完成后需要重新设置RTC闹钟以达到同步的目的
			HalResetRtcAlarm();

			EXTI_ClearITPendingBit (EXTI_IT_Pin0);  // 清除红外运动探测输出引脚中断标志
			RTC_ClearITPendingBit (RTC_IT_ALRA);    // 清除闹钟中断标志
		}

        HAL_MCU_ENTER_SLEEP();   // 进入睡眠模式，等待硬件唤醒
		HAL_MCU_EXIT_SLEEP();    // 退出睡眠模式，并进行相关必要的配置程序
        HalTIM4_Config();
        disableInterrupts();
        HalRFInit();
        HalRFSetChannel(HalRF1, device_config.rf_channel);
        enableInterrupts();
	}
}
/*
 *******************************************************************************
 @brief   主函数

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
int main(void) {
	SystemClockInit (CLK_TYPE_HSE);
	HalFlashInit();
	_hw_board_random_init();
	disableInterrupts();
	spi_init();
	imd_config_load();
	HalRFInit();
	HalRFSetChannel(HalRF1, device_config.rf_channel);
	Hal_led_init();

	enableInterrupts();

	LinkInit();
	LinkSetAddress((int) device_config.net_config.LinkAddr);   // 设置设备地址
	LinkSetNetId((int) device_config.net_config.LinkNetId);    // 设置网络地址
	LinkSetAreaId((int) device_config.net_config.LinkArealId); // 设置区域地址
	LinkSetReceiveCallback(rf_data_receive_handler);

	HalTIM4_Config();
	ConfigLoop();  // 等待rf配置

    disableInterrupts();
	HalIRSensorInit();
    EXTI_ClearITPendingBit(EXTI_IT_Pin0);
	HalRtcInit();

	if (SET == RTC_GetFlagStatus(RTC_FLAG_ALRAF)) {
		RTC_ClearITPendingBit (RTC_IT_ALRA);
	}
	HalResetRtcAlarm();
	HalTIM4_Config();
    enableInterrupts();

	MainLooper();  // 主循环
}
/*
 *******************************************************************************
 @brief   配置等待循环,主要用于开机或复位后进行一定时间的等待，本函数内不做任何的操作
 但是，rf如果收到配置数据会去调用rf数据的回调函数来设置系统参数，每次收到正确的指令后，
 超时定时器重新启动，计数值恢复到CONFIG_TIME_RESTART

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void ConfigLoop(void) {
	static uint32_t time_last = 0;

	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);
	HalRestartRunTimer(LED_FlashTimerID, LED_Flash_Interval, RT_TP_MSECOND);
	while (1) {
		if (HalgetRunTimerCnt(LED_FlashTimerID) == 0) {
			HalRestartRunTimer(LED_FlashTimerID, LED_Flash_Interval, RT_TP_MSECOND);
			HAL_LED_RED_Toggle();
		}

		if (time_last != HalgetRunTimerCnt(GenRunTimerID)) {
			time_last = HalgetRunTimerCnt(GenRunTimerID);
		}
		if (HalgetRunTimerCnt(GenRunTimerID) == 0) {
			break;
		}

		if (ifEndConfigFlag == true) {
			ifEndConfigFlag = false;
			break;
		}
	}

	HAL_LED_RED_OFF();
    if(HalIR_GetOutPutStu() == true){
      ifIrWakeuped = true;
    }
}
/*
 *******************************************************************************
 @brief    rf数据接收处理回调函数

 @params   message -- 接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
static void rf_data_receive_handler(struct LinkMessage *message) {

	SourceAddress = NTOHS(message->srcAddr);

	switch (message->type) {

// 扫描设备
	case LINK_DEVICE_SCAN:
		imd_rf_respond_scan_message();
		break;

		// 设置网络参数
	case MESSAGE_TP_SET_NP_REQ:
		imd_rf_setnetPar_deal(message);
		break;

		// 读取网络参数
	case MESSAGE_TP_GET_NP_REQ:
		imd_rf_getnetPar_deal(message);
		break;

		// 设置运动探测和灯控制参数
	case MESSAGE_TP_SET_MTP_REQ:
		imd_rf_setlightPar_deal(message);
		break;

		// 获取运动探测和灯控参数
	case MESSAGE_TP_GET_MTP_REQ:
		imd_rf_getlightPar_deal(message);
		break;

		// 设置rf信道
	case LINK_SET_CHANNEL:
		imd_rf_setrfChannel(message);
		break;

		// 退出配置循环
	case MESSAGE_TP_TERMINAL_CONFIG_REQ:
		ifEndConfigFlag = true;
		break;

		// 空中升级
	case LINK_ENTER_BOOTLOADER:
		imd_rf_enterBootloader(message);
		break;

	default:
		break;
	}
}
/*
 *******************************************************************************
 @brief    rf回复扫描命令

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_respond_scan_message(void) {
	uint32_t rand_value;
	ScanResponse_t *respondStructer;
	uint8_t buffer[sizeof(ScanResponse_t)];

	Hal_RfTxComplete = 0;

	rand_value = hw_board_random_get();
	rand_value = (rand_value % 100) + (rand_value * 100 % 2000);
	HalRestartRunTimer(FunctionTimerID, rand_value, RT_TP_MSECOND);
	while (1) {
		if (HalgetRunTimerCnt(FunctionTimerID) == 0) {
			break;
		} else {
			HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
		}
	}

// 返回帧
	respondStructer = (ScanResponse_t *) buffer;

	HW_MCU_READ_DEVICE_ID(respondStructer->uniqueId);
	respondStructer->deviceType = DEV_TP_IM_DETECTER;
	respondStructer->areaId = HTONL(device_config.net_config.LinkArealId);
	respondStructer->netId = HTONS(device_config.net_config.LinkNetId);
	respondStructer->address = HTONS(device_config.net_config.LinkAddr);
	respondStructer->enableRoute = device_config.net_config.enableRoute;
	respondStructer->version.major = APP_VERSION;
	respondStructer->version.minor = APP_SUBVERSION;
	respondStructer->version.revise = APP_REVISION;
	respondStructer->channel = device_config.rf_channel;

	LinkSend(SourceAddress, LINK_DEVICE_SCAN_RESPONSE, buffer, sizeof(buffer));
	// 等待无线信号发送完成
	while (0 == Hal_RfTxComplete) {
		;
	}
	Hal_RfTxComplete = 0;
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    设置网络参数

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_setnetPar_deal(struct LinkMessage *message) {
	uint8_t errorCode = 0;
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;

	if (message->length >= 20) {
		HW_MCU_READ_DEVICE_ID(UID_array);
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {
			HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器

			device_config.net_config.LinkAddr = message->data[12];
			device_config.net_config.LinkAddr <<= 8;
			device_config.net_config.LinkAddr += message->data[13];

			device_config.net_config.LinkNetId = message->data[14];
			device_config.net_config.LinkNetId <<= 8;
			device_config.net_config.LinkNetId += message->data[15];

			device_config.net_config.LinkArealId = message->data[16];
			device_config.net_config.LinkArealId <<= 8;
			device_config.net_config.LinkArealId += message->data[17];
			device_config.net_config.LinkArealId <<= 8;
			device_config.net_config.LinkArealId += message->data[18];
			device_config.net_config.LinkArealId <<= 8;
			device_config.net_config.LinkArealId += message->data[19];
			device_config.net_config.enableRoute = FALSE;

			imd_config_restore();

			LinkSend(SourceAddress, MESSAGE_TP_SET_NP_REQ, &errorCode, 1);
			while (0 == Hal_RfTxComplete) { // 等待无线信号发送完成
				;
			}
			Hal_RfTxComplete = 0;

			LinkSetAddress(device_config.net_config.LinkAddr);   // 设置设备地址
			LinkSetNetId(device_config.net_config.LinkNetId);    // 设置网络地址
			LinkSetAreaId(device_config.net_config.LinkArealId); // 设置区域地址
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    读取网络参数

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_getnetPar_deal(struct LinkMessage *message) {
	uint8_t arrayS[9];
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;

	if (message->length >= 12) {
		HW_MCU_READ_DEVICE_ID(UID_array);
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {
			HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器

			arrayS[0] = 0;   // 错误码
			arrayS[1] = (uint8_t)(device_config.net_config.LinkAddr >> 8);
			arrayS[2] = (uint8_t)(device_config.net_config.LinkAddr);

			arrayS[3] = (uint8_t)(device_config.net_config.LinkNetId >> 8);
			arrayS[4] = (uint8_t)(device_config.net_config.LinkNetId);

			arrayS[5] = (uint8_t)(device_config.net_config.LinkArealId >> 24);
			arrayS[6] = (uint8_t)(device_config.net_config.LinkArealId >> 16);
			arrayS[7] = (uint8_t)(device_config.net_config.LinkArealId >> 8);
			arrayS[8] = (uint8_t)(device_config.net_config.LinkArealId);

			LinkSend(SourceAddress, MESSAGE_TP_GET_NP_REQ, arrayS, 9);
			while (0 == Hal_RfTxComplete) {   // 等待无线信号发送完成
				;
			}
			Hal_RfTxComplete = 0;
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    设置红外灯控制参数

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_setlightPar_deal(struct LinkMessage *message) {
	uint8_t errorCode = 0;
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;

	if (message->length >= 22) {
		HW_MCU_READ_DEVICE_ID(UID_array);
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {
			HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器

			device_config.ctr_config.groupId = message->data[12];  // 前12个字节是用于判断消息目标是否是本设备的唯一序列号

			device_config.ctr_config.Leval = message->data[13];

			device_config.ctr_config.ON_seconds = message->data[14];
			device_config.ctr_config.ON_seconds <<= 8;
			device_config.ctr_config.ON_seconds += message->data[15];
			device_config.ctr_config.ON_seconds <<= 8;
			device_config.ctr_config.ON_seconds += message->data[16];
			device_config.ctr_config.ON_seconds <<= 8;
			device_config.ctr_config.ON_seconds += message->data[17];

			device_config.ctr_config.send_peroid = message->data[18];
			device_config.ctr_config.send_peroid <<= 8;
			device_config.ctr_config.send_peroid += message->data[19];
			device_config.ctr_config.send_peroid <<= 8;
			device_config.ctr_config.send_peroid += message->data[20];
			device_config.ctr_config.send_peroid <<= 8;
			device_config.ctr_config.send_peroid += message->data[21];
			imd_config_restore();

			LinkSend(SourceAddress, MESSAGE_TP_SET_MTP_REQ, &errorCode, 1);
			while (0 == Hal_RfTxComplete) {  // 等待无线信号发送完成
				;
			}
			Hal_RfTxComplete = 0;
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    获取红外灯控制参数

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_getlightPar_deal(struct LinkMessage *message) {
	uint8_t arrayS[11];
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;
	if (message->length >= 12) {
		HW_MCU_READ_DEVICE_ID(UID_array);
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {
			HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器

			arrayS[0] = 0;   // 错误码
			arrayS[1] = (uint8_t)(device_config.ctr_config.groupId);
			arrayS[2] = (uint8_t)(device_config.ctr_config.Leval);

			arrayS[3] = (uint8_t)(device_config.ctr_config.ON_seconds >> 24);
			arrayS[4] = (uint8_t)(device_config.ctr_config.ON_seconds >> 16);
			arrayS[5] = (uint8_t)(device_config.ctr_config.ON_seconds >> 8);
			arrayS[6] = (uint8_t)(device_config.ctr_config.ON_seconds);

			arrayS[7] = (uint8_t)(device_config.ctr_config.send_peroid >> 24);
			arrayS[8] = (uint8_t)(device_config.ctr_config.send_peroid >> 16);
			arrayS[9] = (uint8_t)(device_config.ctr_config.send_peroid >> 8);
			arrayS[10] = (uint8_t)(device_config.ctr_config.send_peroid);

			LinkSend(SourceAddress, MESSAGE_TP_GET_MTP_REQ, arrayS, 11);
			while (0 == Hal_RfTxComplete) {   // 等待无线信号发送完成
				;
			}
			Hal_RfTxComplete = 0;
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    设置RF信道

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_setrfChannel(struct LinkMessage *message) {
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;

	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
	HW_MCU_READ_DEVICE_ID(UID_array);

	if (message->length >= 13) {
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {           // 判断是否与stm8唯一序列号匹配
			if (message->data[12] < HAL_RF_CHANNEL_NB) {            // 判断信道值是否合法
				device_config.rf_channel = (HalRFChannel_t) message->data[12];       // 读取并设置信道
				imd_config_restore();

				LinkSend(SourceAddress, LINK_SET_CHANNEL_RES, (uint8_t *) (&device_config.rf_channel), 1);
				while (0 == Hal_RfTxComplete) {       // 等待无线信号发送完成
					;
				}
				Hal_RfTxComplete = 0;
				disableInterrupts();
				HalRFInit();
				HalRFSetChannel(HalRF1, device_config.rf_channel);
				enableInterrupts();
			}
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    进入Bootloader

 @params   message -- rf接收到的消息指针

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_rf_enterBootloader(struct LinkMessage *message) {
	hw_mcu_unique_id_t UID_array;

	Hal_RfTxComplete = 0;

	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
	HW_MCU_READ_DEVICE_ID(UID_array);

	if (message->length >= 13) {
		if (strncmp((char const *) UID_array, (char const *) message->data, 12) == 0) {           // 判断是否与stm8唯一序列号匹配
			if (message->data[12] < HAL_RF_CHANNEL_NB) {            // 判断信道值是否合法
				device_config.boot_rf_channel = (HalRFChannel_t) message->data[12];       // 读取并设置信道
				imd_config_restore();

				LinkSend(SourceAddress, LINK_ENTER_BOOTLOADER_RES, (uint8_t *) (&device_config.boot_rf_channel), 1);
				while (0 == Hal_RfTxComplete) {       // 等待无线信号发送完成
					;
				}
				Hal_RfTxComplete = 0;
				HAL_MCU_REBOOT();
			}
		}
	}
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);  // 重设等待配置超时定时器
}
/*
 *******************************************************************************
 @brief    加载设备参数

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_config_load(void) {
	HalFlashRead(HAL_PAR_CONFIG_BEGIN_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));

	if (device_config.init_flag != FLASH_INIT_FLAG_VALUE) {
		device_config.init_flag = FLASH_INIT_FLAG_VALUE;

		device_config.rf_channel = IMD_RF_CHANNEL_DEFAULT;

		device_config.net_config.LinkAddr = IMD_LINK_ADDR_DEFAULT;
		device_config.net_config.LinkArealId = IMD_LINK_AREAL_ID_DEFAULT;
		device_config.net_config.LinkNetId = IMD_LINK_NET_ADDR_DEFAULT;
		device_config.net_config.enableRoute = FALSE;

		device_config.ctr_config.Leval = IMD_LIGHT_LV_DEFAULT;
		device_config.ctr_config.groupId = IMD_LINK_GROUP_ID_DEFAULT;
		device_config.ctr_config.ON_seconds = IMD_LIGHT_ON_SECOND_DEFAULT;
		device_config.ctr_config.send_peroid = IMD_LIGHT_SEND_PERIOD_DEFAULT;
		device_config.boot_rf_channel = IMD_BOOT_RF_CHANNEL_DEFAULT;

		HalFlashWrite(HAL_PAR_CONFIG_BEGIN_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));
	} else {
		;
	}
}
/*
 *******************************************************************************
 @brief    保存设备参数

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void imd_config_restore(void) {
	HalFlashWrite(HAL_PAR_CONFIG_BEGIN_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	while (1)
	{
	}
}
#endif

