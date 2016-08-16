/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "link.h"
#include <string.h>
#include <stdlib.h>
#include "hal_rf.h"

/*
 *******************************************************************************
 *                                  MACROS                                  
 *******************************************************************************
 */
#define LINK_HTONS(n) (n)
#define LINK_NTOHS(n) (n)
#define LINK_HTONL(n) (n)
#define LINK_NTOHL(n) (n)

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
struct LinkPacket
{
  uint8_t      length;
  LinkAreaId_t areaId;
  LinkNetId_t  netId;
  LinkAddr_t   dstAddr;
  LinkAddr_t   srcAddr;
  uint8_t      transId;
  uint8_t      type;
  uint8_t      data[0];
};

struct PacketCharacteristic
{
  
  LinkAddr_t dstAddr;
  LinkAddr_t srcAddr;
  uint8_t    transId;
  uint8_t    type;
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
  struct PacketCharacteristic lastPacketCharacteristic;
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
static void RFRawDataInput(uint8_t *data, uint16_t length);
static bool IsNewPacket(struct LinkPacket *pkt);
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
  memset(&LinkSelf.lastPacketCharacteristic, 0, sizeof(LinkSelf.lastPacketCharacteristic));
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
    
    packet->length  = size;
    packet->areaId  = LINK_HTONL(LinkSelf.areaId);
    packet->netId   = LINK_HTONS(LinkSelf.netId);
    packet->dstAddr = LINK_HTONS(addr);
    packet->srcAddr = LINK_HTONS(LinkSelf.address);
    packet->transId = LinkSelf.transId++;
    packet->type    = type;
    
    if ( (NULL != data) && (length > 0) )
      memcpy(packet->data, data, length);
    
    buffer[size - 1] = DataChecksum(buffer, size - 1);
    
    RFRawDataOutput(buffer, size);
    
    free(buffer);
    
    return length;
  }
  
  return 0;
}

extern int HalRFRxFlag;

void LinkInternalProcess(void)
{
  uint8_t *buffer, size = 64, length;
  
  if (HalRFRxFlag)
  {
    HalRFRxFlag = FALSE;
    
    buffer = (uint8_t *)malloc(size);
    
    if (buffer != NULL)
    {
      length = HalRFReceive(HalRF1, buffer, size);
      
      RFRawDataInput(buffer, length);      
      free(buffer);
    }
  }
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
static uint16_t RFRawDataOutput(uint8_t *data, uint16_t length)
{
  if (RF_MAX_PAYLOAD >= length)
  {   
    HalRFSend(HalRF1, data, length);
    return length;
  }
  
  return 0;
}

static void RFRawDataInput(uint8_t *data, uint16_t length)
{
  struct LinkPacket  *packet;
  struct LinkMessage  message;
  
  
  if (data != NULL)
  { 
    packet = (struct LinkPacket *)data;

    if ( data[packet->length - 1] != DataChecksum(data, packet->length - 1) ) return;
    
    /* 地域码验证 */
    if ( (LinkSelf.areaId == LINK_NTOHS(packet->areaId)) || (BROADCAST_AREA_ID == LINK_NTOHS(packet->areaId)) )
    {
      //TODO : Do something while area id verify success.
      
      /* 网络号验证 */
      if ( (LinkSelf.netId == LINK_NTOHS(packet->netId)) || (BROADCAST_NET_ID == LINK_NTOHS(packet->netId)) )
      {
        if ( !IsNewPacket(packet) ) return;
        
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

static bool IsNewPacket(struct LinkPacket *pkt)
{
  bool isNew = FALSE;
 
  
  if ( (LinkSelf.lastPacketCharacteristic.dstAddr != LINK_NTOHS(pkt->dstAddr)) || 
       (LinkSelf.lastPacketCharacteristic.srcAddr != LINK_NTOHS(pkt->srcAddr)) ||
       (LinkSelf.lastPacketCharacteristic.transId != pkt->transId)             ||
       (LinkSelf.lastPacketCharacteristic.type    != pkt->type) )
  {
    isNew = TRUE;
    
    LinkSelf.lastPacketCharacteristic.dstAddr   = LINK_NTOHS(pkt->dstAddr);
    LinkSelf.lastPacketCharacteristic.srcAddr   = LINK_NTOHS(pkt->srcAddr);
    LinkSelf.lastPacketCharacteristic.transId   = pkt->transId;
    LinkSelf.lastPacketCharacteristic.type      = pkt->type;
  }
  
  return isNew;
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