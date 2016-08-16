/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "bootloader.h"
#include "frame.h"
#include "port.h"
#include <string.h>

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
typedef enum
{
  /* Indicate Type */
  INDICATE_BL_START,
  
  /* Command Type*/
  CMD_GET_MEM_SIZE,
  CMD_START_DOWNLOAD,
  CMD_UPDATE,
  
  /* Data Type*/
  DATA_CODE,
  
  /* ACK Type*/
  ACK_OK,
  ACK_MEM_SIZE,
  ACK_START,
  ACK_CODE,
  ACK_UPDATE,
  ACK_SIZE_ERROR,
  ACK_LENGTH_ERROR,
  ACK_DATA_ERROR,
  ACK_CRC_ERROR,
  ACK_UNKNOWN_PKT_TYPE,
  ACK_UNKNOWN_CMD_TYPE,
  ACK_UNKNOWN_DATA_TYPE,
}BLPacketType_t;

struct BLPakcet
{
  BLPacketType_t type; 
  uint16_t       length;
  uint8_t        data[0];
};

/*
 *******************************************************************************
 *                              LOCAL VARIABLES                                 
 *******************************************************************************
 */
struct 
{
  uint32_t beginAddr;           /* 应用程序起始地址 */
  uint32_t packetSize;          /* 代码片段大小 */
  uint32_t firmwareSize;        /* 代码总大小 */
  bool     sendIndicate;        /* 当前是否可以发送INDICATE指令给用户 */
  uint32_t seconds;             /* 时间记录 */
  bool     timerRunning;        /* 定时器状态 */
  uint32_t desirePacketId;       /* 代码报文写入编号限制————编号必须连续，否则报错 */
}Bootloader;

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
static void BLTimerInterruptHandler(void);   
//static void BLFrameHandler(enum FrameType type, uint8_t *data, uint16_t length);
static BLResult_t BLSendPacket(BLPacketType_t ackType, uint8_t *data, uint16_t length);
static BLResult_t BLGetMemSizeCmdHandler(uint8_t *data, uint16_t length);
static BLResult_t BLUpdateCmdHandler(uint8_t *data, uint16_t length);
static BLResult_t BLStartDownloadCmdHandler(uint8_t *data, uint16_t length);
static BLResult_t BLDataCodeHandler(uint8_t *data, uint16_t length);
static BLResult_t BLSendPacket(BLPacketType_t type, uint8_t *data, uint16_t length);

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
/*
 *******************************************************************************
  @brief    启动bootloader
 
  @params   无
 
  @return   无
 *******************************************************************************
 */
void BLStartup(void)
{
  Bootloader.sendIndicate  = TRUE;
  Bootloader.seconds       = 0;
  Bootloader.desirePacketId = 0;
 
  /* 初始化与CPU相关部分 */
  BLPortInit();

//  FrameInit();
//  FrameRegister(BLLowDataSend, BLFrameHandler);
  
  BLStartTimer(BLTimerInterruptHandler);
  Bootloader.timerRunning = TRUE;
  
  while (TRUE)
  {
    if (Bootloader.sendIndicate)
    {
      Bootloader.sendIndicate = FALSE;
      BLSendPacket(INDICATE_BL_START, NULL, 0);
    }
    
    BL_POLL();
  }
}

/*
 *******************************************************************************
  @brief    1秒定时中断，bootloader在启动后3秒内没有收到指令将启动应用程序
 
  @params   无
 
  @return   无
 *******************************************************************************
 */
static void BLTimerInterruptHandler(void)
{
  Bootloader.seconds++;
  
  
  if (Bootloader.seconds < 3) 
    Bootloader.sendIndicate = TRUE;
  
  if (Bootloader.seconds >= 3) 
  {
    BLStopTimer();
    BLStartApplication();
  }
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
///*
// *******************************************************************************
//  @brief    数据包处理函数
// 
//  @params   type, data, length 数据包类型，数据，数据长度
// 
//  @return   无
// *******************************************************************************
// */
//static void BLFrameHandler(enum FrameType type, uint8_t *data, uint16_t length)
//{
//  struct BLPakcet *pkt = (struct BLPakcet *)data;
//  
//  if (type != IAP_FRAME)
//  {
//    return;
//  }
//  
//  /* 一旦收到用户指令将不再自动启动应用程序，接下来仅仅相应用户指令 */
//  if (Bootloader.timerRunning)
//  {
//    BLStopTimer();
//    Bootloader.timerRunning = FALSE;
//  }
//  
//  switch (pkt->type)
//  {
//  case CMD_GET_MEM_SIZE:
//    BLGetMemSizeCmdHandler(pkt->data, FRAME_NTOHS(pkt->length));
//    break;
//    
//  case CMD_START_DOWNLOAD:
//    BLStartDownloadCmdHandler(pkt->data, FRAME_NTOHS(pkt->length));
//    break;
//    
//  case CMD_UPDATE:
//    BLUpdateCmdHandler(pkt->data, FRAME_NTOHS(pkt->length));
//    break;
//    
//  case DATA_CODE:
//    BLDataCodeHandler(pkt->data, FRAME_NTOHS(pkt->length));
//    break;
//    
//  default:
//    BLSendPacket(ACK_UNKNOWN_PKT_TYPE, NULL, 0);
//  }
//}
/*
 *******************************************************************************
  @brief    数据包处理函数
 
  @params   data, length 数据，数据长度
 
  @return   无
 *******************************************************************************
 */
void BLDataHandler(uint8_t *data, uint16_t length)
{
  struct BLPakcet *pkt = (struct BLPakcet *)data;
  bool recvUserCmd = TRUE;

    
  switch (pkt->type)
  {  
  case CMD_GET_MEM_SIZE:
    BLGetMemSizeCmdHandler(pkt->data, BL_NTOHS(pkt->length));
    break;
     
  case CMD_START_DOWNLOAD:
    BLStartDownloadCmdHandler(pkt->data, BL_NTOHS(pkt->length));
    break;
    
  case CMD_UPDATE:
    BLUpdateCmdHandler(pkt->data, BL_NTOHS(pkt->length));
    break;
    
  case DATA_CODE:
    BLDataCodeHandler(pkt->data, BL_NTOHS(pkt->length));
    break;
    
  default:
    BLSendPacket(ACK_UNKNOWN_PKT_TYPE, NULL, 0);
  case INDICATE_BL_START:
    recvUserCmd = FALSE;
    break;
  }
  
  /* 一旦收到用户指令将不再自动启动应用程序，接下来仅仅相应用户指令 */
  if (recvUserCmd && Bootloader.timerRunning)
  {
    BLStopTimer();
    Bootloader.timerRunning = FALSE;
  }
}
/*
 *******************************************************************************
  @brief    数据包发送
 
  @params   type, data, length 数据包类型，数据，数据长度
 
  @return   见 BLResult_t 定义
 *******************************************************************************
 */
static BLResult_t BLSendPacket(BLPacketType_t type, uint8_t *data, uint16_t length)
{
  struct BLPakcet *pkt;
  uint16_t packetSize;
  
  
  packetSize = sizeof(struct BLPakcet) + length;
  pkt = (struct BLPakcet *)port_malloc(packetSize);
  
  if (pkt != NULL)
  {
    pkt->type = type;
    pkt->length = BL_HTONS(length);
    if ( (data != NULL) && (length > 0) )
    {
      memcpy(pkt->data, data, length);
    }

    BLLowDataSend((uint8_t *)pkt, packetSize);
    
    port_free(pkt);
    return NO_ERROR;
  }
  
  return MEMORY_ERROR;
}

/*
 *******************************************************************************
  @brief    CMD_GET_MEM_SIZE（获取应用程序空间大小）命令处理函数
 
  @params   data, length
 
  @return   见 BLResult_t 定义
 *******************************************************************************
 */
static BLResult_t BLGetMemSizeCmdHandler(uint8_t *data, uint16_t length)
{
HAL_MCU_DATA_ALIGN(1)  
  struct CmdGetMemPakcet
  {
    uint32_t memSize;
  };
  
  struct CmdGetMemPakcet *pkt;
  uint8_t buf[sizeof(struct CmdGetMemPakcet)];
  
  
  pkt = (struct CmdGetMemPakcet *)buf;
  pkt->memSize = BL_NTOHL(APP_MAX_SIZE);
  
  return BLSendPacket(ACK_MEM_SIZE, buf, sizeof(buf));
}

/*
 *******************************************************************************
  @brief    CMD_START_DOWNLOAD（下载代码前的bootloader参数设置）命令处理函数
 
  @params   data, length
 
  @return   见 BLResult_t 定义
 *******************************************************************************
 */
static BLResult_t BLStartDownloadCmdHandler(uint8_t *data, uint16_t length)
{
HAL_MCU_DATA_ALIGN(1)  
  struct CmdStartPakcet
  {
    uint32_t firmwareSize;
    uint32_t packetSize;
  };
  
  struct CmdStartPakcet *pkt;
  
  
  pkt = (struct CmdStartPakcet *)data;
  
  if ( (NULL == pkt) || (length != sizeof(struct CmdStartPakcet)) )
    return BLSendPacket(ACK_DATA_ERROR, NULL, 0);
  
  Bootloader.beginAddr     = APP_BEGIN_ADDRESS;
  Bootloader.firmwareSize  = BL_NTOHL(pkt->firmwareSize);
  Bootloader.packetSize    = BL_NTOHL(pkt->packetSize);
  Bootloader.desirePacketId = 0;
  
  BLFlashErase(Bootloader.beginAddr, Bootloader.firmwareSize);
  
  return BLSendPacket(ACK_START, NULL, 0);
}

/*
 *******************************************************************************
  @brief    CMD_UPDATE（更新应用程序并启动）命令处理函数
 
  @params   data, length
 
  @return   见 BLResult_t 定义
 *******************************************************************************
 */
static BLResult_t BLUpdateCmdHandler(uint8_t *data, uint16_t length)
{
HAL_MCU_DATA_ALIGN(1)  
  struct UpdateResponse
  {
    bool success;
  };

  uint32_t totalPackets;
  uint8_t  buf[sizeof(struct UpdateResponse)];
  
  struct UpdateResponse *response = (struct UpdateResponse *)buf;
  
  response->success = FALSE;
  
  totalPackets = Bootloader.firmwareSize / Bootloader.packetSize;
  if (Bootloader.firmwareSize % Bootloader.packetSize != 0)
    totalPackets += 1;
  
  if (Bootloader.desirePacketId == totalPackets)
  {
    response->success = TRUE;
  }

  BLSendPacket(ACK_UPDATE, buf, sizeof(buf)); 
  
  if (response->success)
  {
    BLStartApplication();
  }
  
  /* Never execute here. */
  return NO_ERROR;
}

/*
 *******************************************************************************
  @brief    DATA_CODE（代码片段）处理函数
 
  @params   data, length
 
  @return   见 BLResult_t 定义
 *******************************************************************************
 */
static BLResult_t BLDataCodeHandler(uint8_t *data, uint16_t length)
{
HAL_MCU_DATA_ALIGN(1)  
  struct DataCodePakcet
  {
    uint32_t packetId;
    uint8_t  codeData[0];
  };
HAL_MCU_DATA_ALIGN(1)  
  struct AckPacket
  {
    uint32_t packetId;
  };
  
  struct DataCodePakcet *pkt;
  uint32_t codeSize;
  uint8_t  buffer[sizeof(struct AckPacket)];
  struct AckPacket *ack;

  
  pkt = (struct DataCodePakcet *)data;
  ack = (struct AckPacket *)buffer;
  
  ack->packetId = BL_HTONL(Bootloader.desirePacketId);
  
  /* 代码报文包编号必须按顺序发 */
  if (Bootloader.desirePacketId == BL_NTOHL(pkt->packetId))
  {
    Bootloader.desirePacketId++;    
    codeSize = length - sizeof(pkt->packetId);       
    BLFlashWrite(Bootloader.beginAddr + (BL_NTOHL(pkt->packetId) * Bootloader.packetSize), pkt->codeData, codeSize);  
  }
  
  /* 返回当前期望代码报文编号，如果与上位机发送的一致表示当前代码报文被正确处理 */
  return BLSendPacket(ACK_CODE, buffer, sizeof(buffer));
}