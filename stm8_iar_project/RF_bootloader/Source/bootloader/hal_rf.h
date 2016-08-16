#ifndef __HAL_RF__
#define __HAL_RF__
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "stm8l15x.h"

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define HAL_RF_1        
#define xHAL_RF_2
  

#define HAL_RF_DATE_RATE_0_5KBPS         1
#define HAL_RF_DATE_RATE_1_5KBPS         2
#define HAL_RF_DATE_RATE_10KBPS          3

#define HAL_RF_DATE_RATE                 HAL_RF_DATE_RATE_10KBPS

#define HAL_RF_FIFO_SIZE                 64

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
enum HalRFDevice
{
#if (defined HAL_RF_1)
  HalRF1,
#endif
  
#if (defined HAL_RF_2)
  HalRF2,
#endif
  HalRFDeviceNum,
};

enum HalRFChannel
{
  HAL_RF_CHANNEL_0,
  HAL_RF_CHANNEL_1,
  HAL_RF_CHANNEL_2,
  HAL_RF_CHANNEL_3,
  HAL_RF_CHANNEL_4,
  HAL_RF_CHANNEL_5,
  HAL_RF_CHANNEL_NB,
};
typedef enum HalRFChannel HalRFChannel_t;

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void HalRFInit(void);

void HalRFSend(enum HalRFDevice rf, unsigned char *data, unsigned char length);

unsigned char HalRFReceive(enum HalRFDevice rf, unsigned char *data, unsigned char size);

int HalRFReadRSSI(enum HalRFDevice rf);

bool HalRFSetChannel(enum HalRFDevice rf, HalRFChannel_t ch);

#endif

