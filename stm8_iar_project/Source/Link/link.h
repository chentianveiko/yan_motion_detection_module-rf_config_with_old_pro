#ifndef __LINK_H__
#define __LINK_H__
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "hal_mcu.h"

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define BROADCAST_ADDRESS 0xFFFF        /* �㲥�豸��ַ */
#define BROADCAST_NET_ID  0xFFFF        /* �㲥������ */
#define BROADCAST_AREA_ID 0xFFFFFFFFUL  /* �㲥������ */

#define RF_MAX_PAYLOAD    64

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
typedef uint16_t LinkAddr_t;
typedef uint16_t LinkNetId_t;
typedef uint32_t LinkAreaId_t;
struct LinkMessage {
	uint8_t type;
	LinkAddr_t srcAddr;
	uint8_t transId;
	uint16_t length;
	uint8_t *data;
};
typedef void (*LinkReceiveCallback)(struct LinkMessage *message);

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void LinkInit(void);

void LinkSetAddress(LinkAddr_t addr);

void LinkSetNetId(LinkNetId_t netId);

void LinkSetAreaId(LinkAreaId_t areaId);

void LinkSetReceiveCallback(LinkReceiveCallback callback);    // 注册数据接收回调函数

void LinkRouteEnable(bool enable);

uint16_t LinkSend(LinkAddr_t addr, uint8_t type, uint8_t *data, uint16_t length);

#endif

