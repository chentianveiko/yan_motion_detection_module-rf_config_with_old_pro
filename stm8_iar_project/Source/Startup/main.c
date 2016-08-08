/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "main.h"

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
/* link packet type */
#define LIGHT_CONTROL_PACKET             0x01
#define LIGHT_IR_CONTROL                 0x03

/*
 *******************************************************************************
 *                             PUBLIC VARIABLES
 *******************************************************************************
 */
extern int Hal_RfTxComplete;

LinkAddr_t SourceAddress;
imd_config_t device_config;

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */

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

void MainLooper(void) {
	bool ifNeedSend = false;
	Hal_RfTxComplete = 0;

	while (1) {
		HAL_MCU_ENTER_SLEEP();   // 进入睡眠模式，等待硬件唤醒
		HAL_MCU_EXIT_SLEEP();    // 退出睡眠模式，并进行相关必要的配置程序

		GetRtcValue();  // 获取当前RTC时间及日期参数

		if (ifIrWakeuped == true) {
			// 当两次唤醒时间间隔超过设定值
			if (HalTimeCompare(RTC_TimeStr, RTC_DateStr) > device_config.ctr_config.send_peroid) {
				ifIrWakeuped = false;
				ifNeedSend = true; // 准备发送灯控信号
			}
		}

		// 判断是否是闹钟唤醒的
		if (true == ifRtcAlarmWakeup) {
			ifRtcAlarmWakeup = false;

			if ((HalIR_GetOutPutStu() == true) || (ifIrWakeuped == true)) {
				ifIrWakeuped = false;
				// 准备发送一个灯控制信号
				ifNeedSend = true;
			}

			// 每次闹钟唤醒后要重新配置闹钟以达到继续唤醒的目的
			HalResetRtcAlarm();
		}

		// 发送灯控信号
		if (ifNeedSend == true) {
			ifNeedSend = false;
			// 保存本次发送的唤醒时间,以用作下次唤醒后判断
			RTC_DateStr_bck.RTC_Date = RTC_DateStr.RTC_Date;
			RTC_DateStr_bck.RTC_Month = RTC_DateStr.RTC_Month;
			RTC_DateStr_bck.RTC_Year = RTC_DateStr.RTC_Year;

			RTC_TimeStr_bck.RTC_Hours = RTC_TimeStr.RTC_Hours;
			RTC_TimeStr_bck.RTC_Minutes = RTC_TimeStr.RTC_Minutes;
			RTC_TimeStr_bck.RTC_Seconds = RTC_TimeStr.RTC_Seconds;

			IrLightControl(device_config.ctr_config.groupId, device_config.ctr_config.Leval, (device_config.ctr_config.ON_seconds) * 1000); // 发送分组灯控制信号，其中10000是灯开的延时，超过这个时间后，回复到灯的前一个状态

			// 等待无线信号发送完成
			while (0 == Hal_RfTxComplete) {
				;
			}
			Hal_RfTxComplete = 0;

			// 发送完成后需要重新设置RTC闹钟以达到同步的目的
			HalResetRtcAlarm();

			EXTI_ClearITPendingBit (EXTI_IT_Pin0);  // 清除红外运动探测输出引脚中断标志
			RTC_ClearITPendingBit (RTC_IT_ALRA);    // 清除闹钟中断标志
		}
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
	SystemClockInit (CLK_TYPE_HSI);
	HalFlashInit();
	_hw_board_random_init();
	disableInterrupts();
	spi_init();
	HalRFInit();
	HalIRSensorInit();
	enableInterrupts();

	imd_config_load();
	LinkInit();
	LinkSetAddress(device_config.net_config.LinkAddr);   // 设置设备地址
	LinkSetNetId(device_config.net_config.LinkNetId);    // 设置网络地址
	LinkSetAreaId(device_config.net_config.LinkArealId); // 设置区域地址
	LinkSetReceiveCallback (rf_data_receive_handler);

	HalRtcInit();
	if (SET == RTC_GetFlagStatus(RTC_FLAG_ALRAF)) {
		RTC_ClearITPendingBit (RTC_IT_ALRA);
	}
	HalResetRtcAlarm();

	ConfigLoop();  // 等待rf配置
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
	HalRestartRunTimer(GenRunTimerID, CONFIG_TIME_RESTART, RT_TP_SECOND);
	while (1) {
		if (HalgetRunTimerCnt(GenRunTimerID) == 0) {
			break;
		}
		nop();
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
void rf_data_receive_handler(struct LinkMessage *message) {

	SourceAddress = NTOHS(message->srcAddr);

	switch (message->type) {
	case LINK_DEVICE_SCAN:
		imd_rf_respond_scan_message();
		break;
	case MESSAGE_TP_SET_NP_REQ:
		imd_rf_setnetPar_deal(message);
		break;
	case MESSAGE_TP_GET_NP_REQ:
		break;
	case MESSAGE_TP_SET_MTP_REQ:
		break;
	case MESSAGE_TP_GET_MTP_REQ:
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
	HalFlashRead(FLASH_INIT_FLAG_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));

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

		HalFlashWrite(FLASH_INIT_FLAG_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));
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
	HalFlashWrite(FLASH_INIT_FLAG_ADDR, (uint8_t *) (&device_config), sizeof(imd_config_t));
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	while (1)
	{
	}
}
#endif

