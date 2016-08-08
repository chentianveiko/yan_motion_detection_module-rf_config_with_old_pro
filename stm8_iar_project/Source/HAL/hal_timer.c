#include "hal_timer.h"

runTimer_def runTimerArray[MAX_RUN_TIMER_NUM];

/**
 * @brief  Wait 1 sec for LSE stabilization .
 * @param  None.
 * @retval None.
 * Note : TIM4 is configured for a system clock = 2MHz
 */
void LSE_StabTime(void) {

	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

	/* Configure TIM4 to generate an update event each 1 s */
	TIM4_TimeBaseInit(TIM4_Prescaler_16384, 123);
	/* Clear update flag */
	TIM4_ClearFlag (TIM4_FLAG_Update);

	/* Enable TIM4 */
	TIM4_Cmd (ENABLE);

	/* Wait 1 sec */
	while (TIM4_GetFlagStatus(TIM4_FLAG_Update) == RESET)
		;

	TIM4_ClearFlag(TIM4_FLAG_Update);

	/* Disable TIM4 */
	TIM4_Cmd (DISABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);
}

/*
 *********************************************************************************
 函数功能: 配置定时器4,用于超时定时器计时,TIM4的计时最小间隔设为2ms
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalTIM4_Config(void) {
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
	/* Time base configuration */
	TIM4_TimeBaseInit(TIM4_Prescaler_128, 250); // 16000000/128*250=2ms
	/* Clear TIM4 update flag */
	TIM4_ClearFlag (TIM4_FLAG_Update);
	/* Enable update interrupt */
	TIM4_ITConfig(TIM4_IT_Update, ENABLE);

	// enable interrupts
	// enableInterrupts();

	/* Enable TIM4 */
	TIM4_Cmd (ENABLE);
}

/*
 *********************************************************************************
 函数功能: 在定时器4的中断回调函数中调用，用于可配置的超时定时器(秒级调用，最小单位为1秒)
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalRunTimerFuncS(void) {
	uint8_t i;

	for (i = 0; i < MAX_RUN_TIMER_NUM; i++) {
		if ((runTimerArray[i].active == true) && (runTimerArray[i].cnt > 0) && (runTimerArray[i].type == RT_TP_SECOND)) {
			runTimerArray[i].cnt--;
		} else {
			runTimerArray[i].active = false;
		}
	}
}
/*
 *********************************************************************************
 函数功能: 在定时器4的中断回调函数中调用，用于可配置的超时定时器(ms级调用，最小单位为2ms)
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalRunTimerFuncMS(void) {
	uint8_t i;

	for (i = 0; i < MAX_RUN_TIMER_NUM; i++) {
		if ((runTimerArray[i].active == true) && (runTimerArray[i].cnt > 0) && (runTimerArray[i].type == RT_TP_MSECOND)) {
			if (runTimerArray[i].cnt >= 2) {
				runTimerArray[i].cnt -= 2;
			} else {
				runTimerArray[i].cnt = 0;
				runTimerArray[i].active = false;
			}
		}
	}
}

/*
 *********************************************************************************
 函数功能: 获取指定ID的超时定时器的cnt值
 函数参数: runTimerID -- 0~(MAX_RUN_TIMER_NUM-1),表征超时定时器在定时器数组中的
 位置
 函数返回: 对应的超时定时器的cnt值
 *********************************************************************************
 */
uint8_t HalgetRunTimerCnt(uint8_t runTimerID) {
	return runTimerArray[runTimerID].cnt;
}

/*
 *********************************************************************************
 函数功能: 启动指定的定时器，并根据seconds的值初始化该定时器
 函数参数: runTimerID -- 0~(MAX_RUN_TIMER_NUM-1),表征超时定时器在定时器数组中的
 位置
 timeset -- 要设定的超时时间，如果是0，则维持原来的值不变
 runtype -- 运行模式(RT_TP_SECOND--以秒为单位运行  RT_TP_MSECOND以毫秒为单位运行)
 函数返回: None
 *********************************************************************************
 */
void HalRestartRunTimer(uint8_t runTimerID, uint8_t timeset, uint8_t runtype) {
	runTimerArray[runTimerID].active = false;
	runTimerArray[runTimerID].type = runtype;
	runTimerArray[runTimerID].cnt = (timeset == 0) ? runTimerArray[runTimerID].cnt : timeset;
	runTimerArray[runTimerID].active = true;
}

/*
 *********************************************************************************
 函数功能: 定时器4中断回调函数，里面运行HalRunTimerFunc作为超时定时器
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalTIM4_interrupt_callback(void) {
	static int t4_i = 0;

	t4_i++;
	HalRunTimerFuncMS();     // 超时计时器--秒级

	if (t4_i >= 500) {
		t4_i = 0;
		HalRunTimerFuncS();  // 超时计时器--秒级
	}
}

