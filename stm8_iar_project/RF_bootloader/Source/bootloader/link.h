#ifndef __LINK_H__
#define __LINK_H__
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
#define BROADCAST_ADDRESS 0xFFFF        /* 广播设备地址 */
#define BROADCAST_NET_ID  0xFFFF        /* 广播网络编号 */
#define BROADCAST_AREA_ID 0xFFFFFFFFUL  /* 广播区域编号 */

#define RF_MAX_PAYLOAD    64

   
/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
typedef uint16_t LinkAddr_t;
typedef uint16_t LinkNetId_t;
typedef uint32_t LinkAreaId_t;
struct LinkMessage
{
  uint8_t    type;
  LinkAddr_t srcAddr;
  uint8_t    transId;
  uint16_t   length;
  uint8_t   *data;
};
typedef void (* LinkReceiveCallback)(struct LinkMessage *message);

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void LinkInit(void);

void LinkSetAddress(LinkAddr_t addr);

void LinkSetNetId(LinkNetId_t netId);

void LinkSetAreaId(LinkAreaId_t areaId);

void LinkSetReceiveCallback(LinkReceiveCallback callback);

uint16_t LinkSend(LinkAddr_t addr, uint8_t type, uint8_t *data, uint16_t length);

void LinkInternalProcess(void);
#endif

