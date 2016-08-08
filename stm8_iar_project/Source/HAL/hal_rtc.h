#ifndef __hal_rtc__h
#define __hal_rtc__h

/*
 *******************************************************************************
 *                               INCLUDE                                 
 *******************************************************************************
 */
#include "stm8l15x.h"
#include "stdbool.h"

/*
 *******************************************************************************
 *                               global   variable                                 
 *******************************************************************************
 */
extern RTC_InitTypeDef RTC_InitStr;
extern RTC_TimeTypeDef RTC_TimeStr;
extern RTC_DateTypeDef RTC_DateStr;
extern RTC_AlarmTypeDef RTC_AlarmStr;
extern RTC_TimeTypeDef RTC_TimeStr_bck;  // 用于记录上次唤醒的时间
extern RTC_DateTypeDef RTC_DateStr_bck;
extern bool ifRtcAlarmWakeup;

/*
 *******************************************************************************
 *                               FUNCTION                                 
 *******************************************************************************
 */
uint32_t max(uint32_t a, uint32_t b);
uint32_t min(uint32_t a, uint32_t b);
void HalResetRtcAlarm(void);
void HalRtcInit(void);
void RTC_Value_Init(void);
uint16_t HalTimeCompare(RTC_TimeTypeDef timStr, RTC_DateTypeDef dataStr);
uint32_t HalTimeGetSeconds(RTC_TimeTypeDef timStr, RTC_DateTypeDef dataStr);
void GetRtcValue(void);

#endif

