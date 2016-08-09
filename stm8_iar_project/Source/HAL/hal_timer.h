#ifndef __hal_timer__h
#define __hal_timer__h

#include "stm8l15x.h"
#include "stdbool.h"

#define MAX_RUN_TIMER_NUM 2  // 用于设置超时定时器的个数
#define GenRunTimerID     0  // 普通超时定时器ID
#define FunctionTimerID   1  // 普通函数中使用的超时定时器

#define RT_TP_SECOND      0   // 超时定时器采用秒为单位
#define RT_TP_MSECOND     1   // 超时定时器采用毫秒为单位

typedef struct {
	bool active;
	bool type;     // 定时类型
	uint32_t cnt;  // 可配置的超时时间(运行时作减计数，单位根据type可能是秒也可能是毫秒)
} runTimer_def;

extern runTimer_def runTimerArray[MAX_RUN_TIMER_NUM];

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void LSE_StabTime(void);
void HalTIM4_Config(void);
void HalRunTimerFuncS(void);
void HalRunTimerFuncMS(void);
void HalTIM4_interrupt_callback(void);
uint32_t HalgetRunTimerCnt(uint8_t runTimerID);
void HalRestartRunTimer(uint8_t runTimerID, uint32_t timeset, uint8_t runtype);

#endif

