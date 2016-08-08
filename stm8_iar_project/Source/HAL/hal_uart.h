
#ifndef __hal_uart__h
#define __hal_uart__h

/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "stm8l15x.h"
#include "stm8l15x_conf.h"
#include "hal_parset.h"

extern bool ifEnabledUart;   // 用于判断是否使能了串口工作，如果没有，在进入低功耗模式的时候就不用做相关的处理

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
// USART PORT DEFINE
#define UART_CONFIG                   USART1  
#define UART_CONFIG_GPIO              GPIOC
#define UART_CONFIG_CLK               CLK_Peripheral_USART1
#define UART_CONFIG_RxPin             GPIO_Pin_2
#define UART_CONFIG_TxPin             GPIO_Pin_3
#define UART_CONFIG_BaudRate          (9600)
#define UART_CONFIG_WordLength        USART_WordLength_8b
#define UART_CONFIG_StopBits          USART_StopBits_1
#define UART_CONFIG_Parity            USART_Parity_No
#define UART_CONFIG_Mode              (USART_Mode_Rx | USART_Mode_Tx)
   
// USART PARSET STU MACHINE DEFINE
#define PARSET_HEADER1      1
#define PARSET_HEADER2      2
#define PARSET_CMD          3
#define PARSET_LENH         4
#define PARSET_LENL         5
#define PARSET_DATA         6
#define PARSET_XOR          7
#define PARSET_SUM          8
   
#define PARSET_HEADER1_VAL  0x3a    // the value of header
#define PARSET_HEADER2_VAL  0x5a

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalUartConfigInit(void);
void HalUartConfigEnterSleep(void);
void HalUartConfigExitSleep(void);
void HalUartConfigSendByte(unsigned char byteIn);
void HalUartConfigSendString(unsigned char *str);
void HalUartConfigRX_Callback(unsigned char ch);
void HalUartConfigSendStrLen(unsigned char *str,uint8_t len);











#endif


