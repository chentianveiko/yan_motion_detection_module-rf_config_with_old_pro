/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "hal_mcu.h"
#include "virtual_spi.h"
#include "hal_rf.h"
#include "hal_key.h"
//#include "hal_uart.h"
#include "hal_ir_sensor.h"
#include "hal_flash.h"
#include "../Startup/main.h"

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
/* Board enter sleep mode */
void HalMcuEnterSleep(void) {
	disableInterrupts();
    HalIRSensorInit();
	HalRFEnterSleep (HalRF1);
	spi_enter_sleep();

	TIM4_Cmd (DISABLE);

	/* GPIOs low power mode config */
	GPIO_Init(GPIOA, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOB, GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);

	GPIO_Init(GPIOD, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOE, GPIO_Pin_All, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(GPIOF, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

	enableInterrupts();
	HAL_LED_RED_OFF();

	//SystemClockInit(CLK_TYPE_LSE); // 为了进入Active halt模式，切换时钟到LSE
	SystemClockInit (CLK_TYPE_LSI); // 为了进入Active halt模式，切换时钟到LSE

	//PWR_FastWakeUpCmd(ENABLE);
	//PWR_UltraLowPowerCmd(ENABLE);

	PWR_FastWakeUpCmd(DISABLE);
	PWR_UltraLowPowerCmd(DISABLE);

	halt();  // 因为提前切换了系统时钟到LSE,所以这里进入的是Active-halt模式，可由RTC中断唤醒
}

/* Board exit sleep  mode */
void HalMcuExitSleep(void) {
  disableInterrupts();
	SystemClockInit (CLK_TYPE_HSE); // 进入到正常模式使用HSI作为系统时钟

	spi_exit_sleep();
	HalRFExitSleep (HalRF1);

    HalIRSensorInit();
	HalFlashInit();
	enableInterrupts();
}

/*
 *******************************************************************************
 @brief    使用stm8唯一器件序列号来初始化随机数产生使用的种子

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
void _hw_board_random_init(void) {
	hw_mcu_unique_id_t device_id;
	uint8_t *ptr, i;
	uint32_t seed = 0;
	uint16_t tmp = 0xFFFF;
	int32_t j, flag;

	HW_MCU_READ_DEVICE_ID(device_id);

	ptr = device_id;

	for (i = 0; i < sizeof(hw_mcu_unique_id_t); i++) {
		tmp ^= ptr[i];

		for (j = 0; j < 8; j++) {
			flag = tmp & 0x01;
			tmp >>= 1;

			if (1 == flag)
				tmp ^= 0xA001;
		}
	}

	seed = tmp;

	srand(seed);
}
/*
 *******************************************************************************
 @brief    获取随机数

 @params   none

 @author   Veiko

 @time     2016-8-5

 @return   无
 *******************************************************************************
 */
int32_t hw_board_random_get(void) {
	int32_t xback;

	xback = rand();
	return xback;
}
