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

	TIM4_DeInit();
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
 函数功能: 配置定时器4,用于超时定时器计时,TIM4的计时最小间隔设为1ms
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalTIM4_Config(void) {
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
	TIM4_Cmd (DISABLE);
	TIM4_DeInit();
	/* Time base configuration */
	TIM4_TimeBaseInit(TIM4_Prescaler_512, 250); // 16000000/64*250=8ms
	TIM4_ARRPreloadConfig (ENABLE);
	TIM4_SetAutoreload(250);
	/* Clear TIM4 update flag */
	TIM4_ClearFlag (TIM4_FLAG_Update);
	/* Enable update interrupt */
	TIM4_ITConfig(TIM4_IT_Update, ENABLE);

	/* Enable TIM4 */
	TIM4_Cmd(ENABLE);
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
			runTimerArray[i].cnt = 0;
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
	static uint32_t t4_i = 0;

	for (i = 0; i < MAX_RUN_TIMER_NUM; i++) {
		if ((runTimerArray[i].active == true) && (runTimerArray[i].cnt > 0) && (runTimerArray[i].type == RT_TP_MSECOND)) {
			if (runTimerArray[i].cnt >= 8) {
				runTimerArray[i].cnt -= 8;
			} else {
				runTimerArray[i].cnt = 0;
				runTimerArray[i].active = false;
			}
		}
	}

	t4_i++;

	if (t4_i >= 125) {
		t4_i = 0;
		HalRunTimerFuncS();  // 超时计时器--秒级
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
uint32_t HalgetRunTimerCnt(uint8_t runTimerID) {
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
void HalRestartRunTimer(uint8_t runTimerID, uint32_t timeset, uint8_t runtype) {
	disableInterrupts();
	runTimerArray[runTimerID].active = false;
	runTimerArray[runTimerID].type = runtype;
	runTimerArray[runTimerID].cnt = (timeset == 0) ? runTimerArray[runTimerID].cnt : timeset;
	runTimerArray[runTimerID].active = true;
	enableInterrupts();
}
/*
 *********************************************************************************
 函数功能: 用超时定时器做的阻塞式延时
 函数参数: m_sec -- 延时的毫秒数
 函数返回: None
 *********************************************************************************
 */
void HalRunTimerDelayms(uint32_t m_sec){
  HalRestartRunTimer(RunTimerDelayID,m_sec,RT_TP_MSECOND);
  while(1){
    if(HalgetRunTimerCnt(RunTimerDelayID)==0){
      break;
    }
  }
}

/*
 *********************************************************************************
 函数功能: 定时器4中断回调函数，里面运行HalRunTimerFunc作为超时定时器
 函数参数: None
 函数返回: None
 *********************************************************************************
 */
void HalTIM4_interrupt_callback(void) {
	HalRunTimerFuncMS();     // 超时计时器--秒级
}

