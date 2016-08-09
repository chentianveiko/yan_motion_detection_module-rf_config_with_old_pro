/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "link.h"
#include <string.h>
#include <stdlib.h>
#include "hal_rf.h"
//#include "common.h"
//#include "srpan.h"

/*
 *******************************************************************************
 *                                  MACROS
 *******************************************************************************
 */
#define LINK_HTONS(n) HTONS(n)
#define LINK_NTOHS(n) NTOHS(n)
#define LINK_HTONL(n) HTONL(n)
#define LINK_NTOHL(n) NTOHL(n)

/*
 *******************************************************************************
 *                                  TYPEDEFS
 *******************************************************************************
 */
struct LinkPacket
{
  uint8_t          length;
  LinkAreaId_t     areaId;
  LinkNetId_t      netId;
  LinkAddr_t       dstAddr;
  LinkAddr_t       srcAddr;
  uint8_t          transId;
  uint8_t          type;
  uint8_t          data[0];
  /* 检验位：紧跟在data后面 */
};


/*
 *******************************************************************************
 *                              LOCAL VARIABLES
 *******************************************************************************
 */
static struct
{
  LinkAddr_t                  address;
  LinkNetId_t                 netId;
  LinkAreaId_t                areaId;
  LinkReceiveCallback         receiveCallback;
  uint8_t                     transId;
  bool                        rfTransComplete;
}LinkSelf;

/*
 *******************************************************************************
 *                              LOCAL VARIABLES
 *******************************************************************************
 */

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
static uint16_t RFRawDataOutput(uint8_t *data, uint16_t length);
void RFRawDataInput(uint8_t *data, uint16_t length);
static uint8_t DataChecksum(uint8_t *data, uint8_t length);

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
void LinkInit(void)
{
  LinkSelf.receiveCallback = NULL;
  LinkSelf.transId         = 1;
  LinkSelf.rfTransComplete = TRUE;

}

void LinkSetAddress(LinkAddr_t addr)
{
  LinkSelf.address = addr;
}

void LinkSetNetId(LinkNetId_t netId)
{
  LinkSelf.netId = netId;
}

void LinkSetAreaId(LinkAreaId_t areaId)
{
  LinkSelf.areaId = areaId;
}

void LinkSetReceiveCallback(LinkReceiveCallback callback)
{
  LinkSelf.receiveCallback = callback;
}


uint16_t LinkSend(LinkAddr_t addr, uint8_t type, uint8_t *data, uint16_t length)
{
  struct LinkPacket *packet;
  uint8_t *buffer;
  uint16_t size;

  size = sizeof(struct LinkPacket) + length + 1;
  buffer = (uint8_t *)malloc(size);

  if (size > RF_MAX_PAYLOAD) return 0;

  if (buffer != NULL)
  {
    packet = (struct LinkPacket *)buffer;

    packet->length           = size;
    packet->areaId           = LINK_HTONL(LinkSelf.areaId);
    packet->netId            = LINK_HTONS(LinkSelf.netId);
    packet->dstAddr          = LINK_HTONS(addr);
    packet->srcAddr          = LINK_HTONS(LinkSelf.address);
    packet->transId          = LinkSelf.transId++;
    packet->type             = type;

    if ( (NULL != data) && (length > 0) )
      memcpy(packet->data, data, length);

    buffer[size - 1] = DataChecksum(buffer, size - 1);

    RFRawDataOutput(buffer, size);

    free(buffer);

    return length;
  }

  return 0;
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
//RFRawDataInput(buffer, length);



static uint16_t RFRawDataOutput(uint8_t *data, uint16_t length)
{
  if (RF_MAX_PAYLOAD >= length)
  {
//    if (LinkSelf.rfTransComplete)
    {
      HalRFSend(HalRF1, data, length);
//      LinkSelf.rfTransComplete = FALSE;
      return length;
    }
//    else
//    {
//      memcpy(tx_buf.buffer, data, length);
//      tx_buf.length = length;
//      ETOSTriggerEvent(EVENT_DELAY_TRANS, 0, NULL, TRUE);
//    }
  }

  return 0;
}

void RFRawDataInput(uint8_t *data, uint16_t length)
{
  struct LinkPacket  *packet;
  struct LinkMessage  message;


  if (data != NULL)
  {
    packet = (struct LinkPacket *)data;

    if (packet->length < 1) return ;
    /* checksum verify */
    if ( data[packet->length - 1] != DataChecksum(data, packet->length - 1) ) return;

    /* 地域码验证 */
    if ( (LinkSelf.areaId == LINK_NTOHL(packet->areaId)) || (BROADCAST_AREA_ID == LINK_NTOHL(packet->areaId)) )
    {
      /* 网络号验证 */
      if ( (LinkSelf.netId == LINK_NTOHS(packet->netId)) || (BROADCAST_NET_ID == LINK_NTOHS(packet->netId)) )
      {
        /* 地址验证 */
        if ( (LinkSelf.address == LINK_NTOHS(packet->dstAddr)) || (BROADCAST_ADDRESS == LINK_NTOHS(packet->dstAddr)) )
        {
          message.srcAddr = LINK_NTOHS(packet->srcAddr);
          message.transId = packet->transId;
          message.type    = packet->type;
          message.length  = packet->length - sizeof(struct LinkPacket) - 1;
          message.data    = packet->data;

          if (LinkSelf.receiveCallback != NULL)
          {
            LinkSelf.receiveCallback(&message);
          }
        }
      }
    }
  }
}


static uint8_t DataChecksum(uint8_t *data, uint8_t length)
{
  uint8_t checksum = 0;
  uint8_t i;

  if ( (data != NULL) && (length != 0) )
  {
    for (i = 0; i < length; i++)
      checksum += data[i];
  }

  return checksum;
}
