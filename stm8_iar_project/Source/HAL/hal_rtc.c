#include "hal_rtc.h"
#include "hal_timer.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include "hal_parset.h"
//#include "hal_uart.h"
#include "../Startup/main.h"

RTC_InitTypeDef RTC_InitStr;
RTC_TimeTypeDef RTC_TimeStr;
RTC_DateTypeDef RTC_DateStr;
RTC_AlarmTypeDef RTC_AlarmStr;

RTC_TimeTypeDef RTC_TimeStr_bck;  // 用于记录上次唤醒的时间
RTC_DateTypeDef RTC_DateStr_bck;  // 用于记录上次唤醒的日期

bool ifRtcAlarmWakeup = false;  // 为true表示是rtc闹钟唤醒的系统

/*
 *********************************************************************************
 函数功能: 获取两数的最大值
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
uint32_t max(uint32_t a, uint32_t b) {
	return a > b ? a : b;
}

/*
 *********************************************************************************
 函数功能: 获取两数的最小值
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
uint32_t min(uint32_t a, uint32_t b) {
	return a > b ? b : a;
}

/*
 *********************************************************************************
 函数功能: 根据时间间隔值重设RTC的闹钟时间，顺延闹钟唤醒中断
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalResetRtcAlarm(void) {
	uint32_t secondtmp = 0, minutetmp = 0, hourtmp = 0;

	disableInterrupts();
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);  // 使能RTC外设

	RTC_ITConfig(RTC_IT_ALRA, DISABLE);
	RTC_AlarmCmd (DISABLE);

	// 获取时间
	while (RTC_WaitForSynchro() != SUCCESS)
		;
	RTC_GetTime(RTC_Format_BIN, &(RTC_TimeStr));

	secondtmp = device_config.ctr_config.send_peroid + RTC_TimeStr.RTC_Seconds;
	minutetmp = secondtmp / 60 + RTC_TimeStr.RTC_Minutes;

	RTC_AlarmStr.RTC_AlarmTime.RTC_Seconds = secondtmp % 60;  // 配置闹钟的秒
	// 配置闹钟的小时
	hourtmp = minutetmp / 60 + RTC_TimeStr.RTC_Hours;
	RTC_AlarmStr.RTC_AlarmTime.RTC_Hours = hourtmp % 24;
	// 配置闹钟分钟
	RTC_AlarmStr.RTC_AlarmTime.RTC_Minutes = minutetmp % 60;
	RTC_AlarmStr.RTC_AlarmTime.RTC_H12 = RTC_TimeStr.RTC_H12;

	RTC_AlarmStr.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;
	RTC_SetAlarm(RTC_Format_BIN, &RTC_AlarmStr);

	RTC_ITConfig(RTC_IT_ALRA, ENABLE);
	RTC_AlarmCmd (ENABLE);
	enableInterrupts();
	ifRtcAlarmWakeup = false;
}

/*
 *********************************************************************************
 函数功能: 获取当前时刻和上次唤醒的时刻的秒数间隔(当年不相等|年且月不相等|年且月且天不相等|
 年且月且天且小时不相等时直接发送超过当前设定的值的秒数以让无线发送信号）
 函数参数: 当前时间的时间结构体及当前时间的日期结构体
 函数返回: 秒数时间间隔
 *********************************************************************************
 */
uint16_t HalTimeCompare(RTC_TimeTypeDef timStr, RTC_DateTypeDef dataStr) {
	uint32_t calcTmp_now, calcTmp_old;

	calcTmp_now = HalTimeGetSeconds(timStr, dataStr);
	calcTmp_old = HalTimeGetSeconds(RTC_TimeStr_bck, RTC_DateStr_bck);
	calcTmp_old = max(calcTmp_now, calcTmp_old) - min(calcTmp_now, calcTmp_old);

	return calcTmp_old;  // 返回实际相差的秒数

	// 获取实际间隔时间
	/*if (dataStr.RTC_Year == RTC_DateStr_bck.RTC_Year) {
	 if (dataStr.RTC_Month == RTC_DateStr_bck.RTC_Month) {
	 if (dataStr.RTC_Date == RTC_DateStr_bck.RTC_Date) {
	 if (timStr.RTC_Hours == RTC_TimeStr_bck.RTC_Hours) {
	 calcTmp_now = timStr.RTC_Minutes * 60 + timStr.RTC_Seconds;
	 calcTmp_old = RTC_TimeStr_bck.RTC_Minutes * 60 + RTC_TimeStr_bck.RTC_Seconds;

	 return max(calcTmp_now, calcTmp_old) - min(calcTmp_now, calcTmp_old);  // 返回实际相差的秒数
	 } else {
	 return (device_config.ctr_config.send_peroid + 1);
	 }
	 } else {
	 return (device_config.ctr_config.send_peroid + 1);
	 }
	 } else {
	 return (device_config.ctr_config.send_peroid + 1);
	 }
	 } else {
	 return (device_config.ctr_config.send_peroid + 1);
	 }*/
}
uint32_t HalTimeGetSeconds(RTC_TimeTypeDef timStr, RTC_DateTypeDef dataStr) {
	uint32_t seconds;

	seconds = dataStr.RTC_Year * 12 + dataStr.RTC_Month;  // 获取月份总数
	seconds = seconds * 31 + dataStr.RTC_Date;  // 获取日期总数
	seconds = seconds * 24 + timStr.RTC_Hours;  // 获取小时总数
	seconds = seconds * 60 + timStr.RTC_Minutes;  // 获取分钟总数
	seconds = seconds * 60 + timStr.RTC_Seconds;  // 获取秒总为九

	return seconds;
}

/*
 *********************************************************************************
 函数功能: 初始化RTC及相关的全局变量
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalRtcInit(void) {
	RTC_TimeStr_bck.RTC_Hours = 0;
	RTC_TimeStr_bck.RTC_Minutes = 0;
	RTC_TimeStr_bck.RTC_Seconds = 0;

	RTC_DateStr_bck.RTC_Year = 0;
	RTC_DateStr_bck.RTC_Month = RTC_Month_January;
	RTC_DateStr_bck.RTC_Date = 0;

	if (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET) {
		/* Enable LSE */
		CLK_LSEConfig (CLK_LSE_ON);
		/* Wait for LSE clock to be ready */
		while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET)
			;
		LSE_StabTime();

		/* Select LSE (32.768 KHz) as RTC clock source */
		CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);
	}
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);

	/* Calendar Configuration */
	RTC_Value_Init();
}

/*
 *********************************************************************************
 函数功能: 配置RTC内的时间，日期及闹钟
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void RTC_Value_Init(void) {
	RTC_InitStr.RTC_HourFormat = RTC_HourFormat_24;
	RTC_InitStr.RTC_AsynchPrediv = 0x7F;
	RTC_InitStr.RTC_SynchPrediv = 0x00FF;
	RTC_Init(&RTC_InitStr);

	RTC_DateStructInit(&RTC_DateStr);
	RTC_DateStr.RTC_WeekDay = RTC_Weekday_Friday;
	RTC_DateStr.RTC_Date = 13;
	RTC_DateStr.RTC_Month = RTC_Month_May;
	RTC_DateStr.RTC_Year = 11;
	RTC_SetDate(RTC_Format_BIN, &RTC_DateStr);

	RTC_TimeStructInit(&RTC_TimeStr);
	RTC_TimeStr.RTC_Hours = 23;
	RTC_TimeStr.RTC_Minutes = 00;
	RTC_TimeStr.RTC_Seconds = 00;
	RTC_SetTime(RTC_Format_BIN, &RTC_TimeStr);

	//GetRtcValue();

	HalResetRtcAlarm();
}

/*
 *********************************************************************************
 函数功能: 获取RTC的时间及日期传递给全局变量
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void GetRtcValue(void) {
	// 获取时间
	while (RTC_WaitForSynchro() != SUCCESS)
		;
	RTC_GetTime(RTC_Format_BCD, &RTC_TimeStr);

	// 获取日期
	/* Wait until the calendar is synchronized */
	while (RTC_WaitForSynchro() != SUCCESS)
		;
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStr);
}

