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
  uint32_t beginAddr;           /* Ӧ�ó�����ʼ��ַ */
  uint32_t packetSize;          /* ����Ƭ�δ�С */
  uint32_t firmwareSize;        /* �����ܴ�С */
  bool     sendIndicate;        /* ��ǰ�Ƿ���Է���INDICATEָ����û� */
  uint32_t seconds;             /* ʱ���¼ */
  bool     timerRunning;        /* ��ʱ��״̬ */
  uint32_t desirePacketId;       /* ���뱨��д�������ơ���������ű������������򱨴� */
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
  @brief    ����bootloader
 
  @params   ��
 
  @return   ��
 *******************************************************************************
 */
void BLStartup(void)
{
  Bootloader.sendIndicate  = TRUE;
  Bootloader.seconds       = 0;
  Bootloader.desirePacketId = 0;
 
  /* ��ʼ����CPU��ز��� */
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
  @brief    1�붨ʱ�жϣ�bootloader��������3����û���յ�ָ�����Ӧ�ó���
 
  @params   ��
 
  @return   ��
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
//  @brief    ���ݰ�������
// 
//  @params   type, data, length ���ݰ����ͣ����ݣ����ݳ���
// 
//  @return   ��
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
//  /* һ���յ��û�ָ������Զ�����Ӧ�ó��򣬽�����������Ӧ�û�ָ�� */
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
  @brief    ���ݰ�������
 
  @params   data, length ���ݣ����ݳ���
 
  @return   ��
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
  
  /* һ���յ��û�ָ������Զ�����Ӧ�ó��򣬽�����������Ӧ�û�ָ�� */
  if (recvUserCmd && Bootloader.timerRunning)
  {
    BLStopTimer();
    Bootloader.timerRunning = FALSE;
  }
}
/*
 *******************************************************************************
  @brief    ���ݰ�����
 
  @params   type, data, length ���ݰ����ͣ����ݣ����ݳ���
 
  @return   �� BLResult_t ����
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
  @brief    CMD_GET_MEM_SIZE����ȡӦ�ó���ռ��С���������
 
  @params   data, length
 
  @return   �� BLResult_t ����
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
  @brief    CMD_START_DOWNLOAD�����ش���ǰ��bootloader�������ã��������
 
  @params   data, length
 
  @return   �� BLResult_t ����
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
  @brief    CMD_UPDATE������Ӧ�ó����������������
 
  @params   data, length
 
  @return   �� BLResult_t ����
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
  @brief    DATA_CODE������Ƭ�Σ�������
 
  @params   data, length
 
  @return   �� BLResult_t ����
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
  
  /* ���뱨�İ���ű��밴˳�� */
  if (Bootloader.desirePacketId == BL_NTOHL(pkt->packetId))
  {
    Bootloader.desirePacketId++;    
    codeSize = length - sizeof(pkt->packetId);       
    BLFlashWrite(Bootloader.beginAddr + (BL_NTOHL(pkt->packetId) * Bootloader.packetSize), pkt->codeData, codeSize);  
  }
  
  /* ���ص�ǰ�������뱨�ı�ţ��������λ�����͵�һ�±�ʾ��ǰ���뱨�ı���ȷ���� */
  return BLSendPacket(ACK_CODE, buffer, sizeof(buffer));
}