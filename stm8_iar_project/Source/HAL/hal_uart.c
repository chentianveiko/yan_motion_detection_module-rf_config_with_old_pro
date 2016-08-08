#include "hal_uart.h"

bool ifEnabledUart = true;

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */

/****************************************************
 * @brief  Configures the Config COM port.
 * @param  None
 * @retval None
 ***************************************************/
void HalUartConfigInit(void) {
	USART_DeInit (UART_CONFIG);
	USART_Cmd(UART_CONFIG, DISABLE);
	// Enable USART clock
	CLK_PeripheralClockConfig((CLK_Peripheral_TypeDef) UART_CONFIG_CLK, ENABLE);

	GPIO_Init(UART_CONFIG_GPIO, UART_CONFIG_RxPin, GPIO_Mode_In_PU_No_IT);
	GPIO_Init(UART_CONFIG_GPIO, UART_CONFIG_TxPin, GPIO_Mode_Out_PP_High_Fast);

	// Configure USART Tx as alternate function push-pull  (software pull up)
	GPIO_ExternalPullUpConfig(UART_CONFIG_GPIO, UART_CONFIG_TxPin, ENABLE);

	// Configure USART Rx as alternate function push-pull  (software pull up)
	GPIO_ExternalPullUpConfig(UART_CONFIG_GPIO, UART_CONFIG_RxPin, ENABLE);

	// USART configuration
	USART_Init(UART_CONFIG, UART_CONFIG_BaudRate, UART_CONFIG_WordLength, UART_CONFIG_StopBits, UART_CONFIG_Parity,
			(USART_Mode_TypeDef) UART_CONFIG_Mode);

	// Enable global interrupts
	enableInterrupts();

	// Enable the RX Interrupt
	USART_ITConfig(UART_CONFIG, USART_IT_RXNE, ENABLE);

	// Disable the TX Interrupt
	USART_ITConfig(UART_CONFIG, USART_IT_TC, DISABLE);

	USART_Cmd(UART_CONFIG, ENABLE);

	ifEnabledUart = true;
}

void HalUartConfigEnterSleep(void) {
	// 配置引脚使其在低功耗模式下节能
	GPIO_Init(UART_CONFIG_GPIO, UART_CONFIG_RxPin, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(UART_CONFIG_GPIO, UART_CONFIG_TxPin, GPIO_Mode_Out_PP_Low_Slow);
	USART_Cmd(UART_CONFIG, DISABLE);
}

void HalUartConfigExitSleep(void) {
	// 从低功耗模式被唤醒，重新配置串口
	HalUartConfigInit();
}

void HalUartConfigSendByte(unsigned char byteIn) {
	// Waitting for the Empty Tx buffer
	while ((USART_GetFlagStatus(UART_CONFIG, USART_FLAG_TC) == RESET))
		;

	// Send thd special byte
	USART_SendData8(UART_CONFIG, byteIn);
}

void HalUartConfigSendString(unsigned char *str) {
	int i = 0;
	while ((*(str + i)) != '\0') {
		HalUartConfigSendByte(*(str + i));
		i++;
	}
}

void HalUartConfigSendStrLen(unsigned char *str, uint8_t len) {
	uint8_t i = 0;
	while (len--) {
		HalUartConfigSendByte(*(str + i));
		i++;
	}
}

void HalUartConfigRX_Callback(unsigned char ch) {
	HalUartConfigSendByte(ch);
}

static char s0[200];
void debug_log(const char *fmt, ...) {
	va_list list;

	va_start(list, fmt);
	vsprintf(s0, fmt, list);
	va_end(list);

	HalUartConfigSendStrLen((uint8_t *) s0, strlen(s0));
}

