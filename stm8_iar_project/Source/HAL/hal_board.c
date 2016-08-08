/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_board.h"
#include "hal_ble.h"
#include "hal_led.h"
#include "stm8l15x.h"
#include <string.h>
#include <stdio.h>

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define USART_DMA_CHANNEL_RX   DMA1_Channel3
#define USART_DMA_CHANNEL_TX   DMA1_Channel0
#define USART_DMA_FLAG_TCRX    DMA1_FLAG_TC3
#define USART_DMA_FLAG_TCTX    DMA1_FLAG_TC0
#define USART_DR_ADDRESS       (uint16_t)USART2->DR  /* USART2 Data register Address */
#define USART_DMA_BUFFER_SIZE  32

/*
 *******************************************************************************
 *                              LOCAL VARIABLES                                 
 *******************************************************************************
 */
static uint8_t HalConsoleTxBuffer[USART_DMA_BUFFER_SIZE], HalConsoleRxBuffer[USART_DMA_BUFFER_SIZE];

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
static void HalConsoleUARTConfigration(void);
static void HalConsoleDMAConfigration(void);

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalBoardInit(void)
{
  HalConsoleUARTConfigration();
  HalConsoleDMAConfigration();
  
  
}

__ATTRIBUTES int putchar(int c)
{
  char ch = c;
  
  HalConsoleWrite((const Byte_t *)&ch, 1);
  return c;
}

/*
 *******************************************************************************
  @brief    Send data via console port
 
  @params   data, length 
 
  @return   none
 *******************************************************************************
 */
void HalConsoleWrite(const Byte_t *data, UInt16_t length)
{
  UInt16_t send_length;
  
  
  if ( (data != DEF_NULL) && (length != 0) )
  {
    DMA_ClearFlag(USART_DMA_FLAG_TCTX);
    
    send_length = sizeof(HalConsoleTxBuffer) >= length ? length : sizeof(HalConsoleTxBuffer);
    memcpy(HalConsoleTxBuffer, data, send_length);
    
    /* DMA channel Tx of USART Configuration */
    DMA_Init(USART_DMA_CHANNEL_TX, (uint16_t)HalConsoleTxBuffer, (uint16_t)USART_DR_ADDRESS,
             send_length, DMA_DIR_MemoryToPeripheral, DMA_Mode_Normal,
             DMA_MemoryIncMode_Inc, DMA_Priority_High, DMA_MemoryDataSize_Byte);
    
    /* Enable the USART Tx DMA channel */
    DMA_Cmd(USART_DMA_CHANNEL_TX, ENABLE);
  }
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
static void HalConsoleUARTConfigration(void)
{
  /* Enable USART clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_USART2, ENABLE);
  
  /* Configure USART Tx as alternate function push-pull  (software pull up)*/
  GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_4, ENABLE);
  
  /* Configure USART Rx as alternate function push-pull  (software pull up)*/
  GPIO_ExternalPullUpConfig(GPIOE, GPIO_Pin_3, ENABLE);
  
  /* USART configuration */
  USART_Init(USART2, 115200, USART_WordLength_8b, USART_StopBits_1,
             USART_Parity_No,(USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));
  
  USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
  USART_Cmd(USART2, DISABLE);
}

static void HalConsoleDMAConfigration(void)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);
  
  /* Deinitialize DMA channels */
  DMA_GlobalDeInit();
  
  DMA_DeInit(USART_DMA_CHANNEL_TX);
  DMA_DeInit(USART_DMA_CHANNEL_RX);
  
  /* DMA channel Rx of USART Configuration */
  DMA_Init(USART_DMA_CHANNEL_RX, (uint16_t)HalConsoleRxBuffer, (uint16_t)USART_DR_ADDRESS,
           sizeof(HalConsoleRxBuffer), DMA_DIR_PeripheralToMemory, DMA_Mode_Normal,
           DMA_MemoryIncMode_Inc, DMA_Priority_Low, DMA_MemoryDataSize_Byte);
  
  
  /* Enable the USART Tx/Rx DMA requests */
  USART_DMACmd(USART2, USART_DMAReq_TX, ENABLE);
  USART_DMACmd(USART2, USART_DMAReq_RX, ENABLE);
  
  /* Global DMA Enable */
  DMA_GlobalCmd(ENABLE);
  
  /* Enable the USART Rx DMA channel */
  DMA_Cmd(USART_DMA_CHANNEL_RX, ENABLE);
  USART_Cmd(USART2, ENABLE);
}

INTERRUPT_HANDLER(TIM2_CC_USART2_RX_IRQHandler, 20)
{
  UInt16_t length;
  
  
  if ( USART_GetITStatus(USART2, USART_IT_IDLE) == SET )
  {
    /*dummy read, clear flags*/
    USART_ReceiveData8(USART2);
    
    length = sizeof(HalConsoleRxBuffer) - DMA_GetCurrDataCounter(USART_DMA_CHANNEL_RX);
//    FrameInput(HalConsoleRxBuffer, length);
    
    DMA_ClearFlag(USART_DMA_FLAG_TCRX);
                  
    DMA_DeInit(USART_DMA_CHANNEL_RX);
    
    /* DMA channel Rx of USART Configuration */
    DMA_Init(USART_DMA_CHANNEL_RX, (uint16_t)HalConsoleRxBuffer, (uint16_t)USART_DR_ADDRESS,
             sizeof(HalConsoleRxBuffer), DMA_DIR_PeripheralToMemory, DMA_Mode_Normal,
             DMA_MemoryIncMode_Inc, DMA_Priority_Low, DMA_MemoryDataSize_Byte);
    
    /* Enable the USART Rx DMA channel */
    DMA_Cmd(USART_DMA_CHANNEL_RX, ENABLE);
  }
}