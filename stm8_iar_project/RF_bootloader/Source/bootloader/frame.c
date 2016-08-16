/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "frame.h"
#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define FRAME_BUFFER_SIZE       256

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
/* 8位机不考虑对齐 */
struct FrameHead
{
  uint16_t begin;
  uint16_t length;
  uint8_t  type;
  uint8_t  trans_id;
};

struct FrameTail
{
  uint16_t crc;
  uint16_t end;
};

   
/*
 *******************************************************************************
 *                              LOCAL VARIABLES                                 
 *******************************************************************************
 */
static FrameOutput_t  FrameOutputFunc;
static FrameProcess_t FrameUserProcessFunc;   
static ringbuffer_t  *FrameRbuffer;

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
static uint16_t FrameCRC16(uint8_t *buffer, uint16_t length);
static uint32_t FrameTake(uint8_t *buffer, uint16_t size);

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
/*
 *******************************************************************************
  @brief    初始化通信层
 
  @params   无
	
  @return   无
 *******************************************************************************
 */ 
void FrameInit(void)
{
  FrameOutputFunc      = NULL;
  FrameUserProcessFunc = NULL;
  
  FrameRbuffer = ringbuffer_create(FRAME_BUFFER_SIZE);
}


/*
 *******************************************************************************
  @brief    发送数据包
 
  @params   type  - 帧类型
	    data  - 发送的报文 
	    length- 报文长度
	
  @return   实际发送的字节数，改善失败返回0
 *******************************************************************************
 */ 
uint16_t FrameSend(enum FrameType type, uint8_t *data, uint16_t length)
{
  uint8_t            *frame_buffer;
  uint16_t            frame_length, size = 0;
  struct FrameHead *head;
  struct FrameTail *tail;
  static uint8_t      trans_id = 0;
  
  
  frame_length = sizeof(struct FrameHead) + length + sizeof(struct FrameTail);
  frame_buffer = malloc(frame_length);
   
  if (frame_buffer != NULL)
  {
    head = (struct FrameHead *)&frame_buffer[0];
    
    head->begin    = FRAME_HTONS(FRAME_BEGIN);
    head->length   = FRAME_HTONS(length);
    head->trans_id = trans_id++;
    head->type     = type;
    
    memcpy(&frame_buffer[sizeof(struct FrameHead)], data, length);
    
    tail = (struct FrameTail *)&frame_buffer[frame_length - sizeof(struct FrameTail)];
    
    tail->crc = FRAME_HTONS(FrameCRC16(frame_buffer, frame_length - sizeof(struct FrameTail)));
    tail->end = FRAME_HTONS(FRAME_END);      
    
    if (FrameOutputFunc != NULL) 
      size = FrameOutputFunc(frame_buffer, frame_length);
    
    free(frame_buffer);
  }
  else
  {
    size = 0;
  }
  
  return size;
}

/*
 *******************************************************************************
  @brief    注册数据底层发送函数及接收回调函数
 
  @params   output  - 底层发送函数
            process - 接收回调函数
 
  @return   无
 *******************************************************************************
 */
void FrameRegister(FrameOutput_t output, FrameProcess_t process)
{
  FrameOutputFunc      = output;
  FrameUserProcessFunc = process;
}

/*
 *******************************************************************************
  @brief    将接收到数据包input进入Frame的内部缓存
 
  @params   data  - 接收的报文 
	    length- 报文长度
 
  @return   缓存进frame内部buffer的数据长度，如果返回0表示frame内部buffer已满
 *******************************************************************************
 */
uint16_t FrameInput(uint8_t *data, uint16_t length)
{
  uint32_t size;
  
  
  size = ringbuffer_put_bytes(FrameRbuffer, data, length);
  
  return size;
}

/*
 *******************************************************************************
  @brief    处理ringbuffer中的数据
 
  @params   none
 
  @return   none
 *******************************************************************************
 */
void FrameDataProcess(void)
{
  uint8_t            *buffer, *data;
  uint16_t            crc, length;
  struct FrameHead *head;
  struct FrameTail *tail;
  
  
  buffer = malloc(FRAME_BUFFER_SIZE);
  
  if (buffer != NULL)
  {
    length = FrameTake(buffer, FRAME_BUFFER_SIZE);
    
    if (length > 0)
    {
      head = (struct FrameHead *)&buffer[0];
      data = (uint8_t *)&buffer[sizeof(struct FrameHead)];
      tail = (struct FrameTail *)&buffer[length - sizeof(struct FrameTail)];
      
      crc = FrameCRC16(buffer, length - sizeof(struct FrameTail));
      
      if (FRAME_NTOHS(tail->crc) == crc)
      {
        if (FrameUserProcessFunc != NULL)
          FrameUserProcessFunc((enum FrameType)head->type, data, FRAME_NTOHS(head->length));
      } 
    }
    
    free(buffer);
  }
}


/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS                                  
 *******************************************************************************
 */
/*
 *******************************************************************************
  @brief    CRC16校验码生成
 
  @params   buffer - 生成校验的数据包
            length - 数据包长度
 
  @return   CRC16校验码
 *******************************************************************************
 */
static uint16_t FrameCRC16(uint8_t *buffer, uint16_t length)
{
  uint16_t tmp = 0xFFFF;
  int32_t  i, j, flag;
  
  
  for (i = 0; i < length; i++)
  {  
    tmp ^= buffer[i];
    
    for (j = 0 ; j < 8; j++)
    {
      flag  = tmp & 0x01;			
      tmp >>= 1;
      
      if (1 == flag) tmp ^= 0xA001;
    }
  }
  
  return tmp;
}

/*
 *******************************************************************************
  @brief    从接收buffer中提取出一帧数据数据，不对数据正确校验，只保证数据有正确
	    的长度及帧头、帧尾
 
  @params   buffer - 接收数据帧的缓存
            size   - 接收数据帧缓存的长度
 
  @return   成功提取出一帧数据返回数据帧的长度，否则返回0
 *******************************************************************************
 */
static uint32_t FrameTake(uint8_t *buffer, uint16_t size)
{
  uint32_t read_bytes, minimum, index, length = 0;
  uint8_t *tmp_buffer;
  bool  find_frame = false;
  struct FrameHead *head;
  struct FrameTail *tail;
  
  
  if ( (NULL == buffer) || (0 == size)) goto __label_out;
  
  tmp_buffer = malloc(FRAME_BUFFER_SIZE);
  if (NULL == tmp_buffer) goto __label_out;
  
  memset(tmp_buffer, 0, FRAME_BUFFER_SIZE);
  
  /* 复制出RingBuffer中已有的数据,RingBuffer中的数据仍然存在 */
  read_bytes = ringbuffer_view_bytes(FrameRbuffer, tmp_buffer, FRAME_BUFFER_SIZE);
  /* 数据帧最小尺寸：数据帧中除用户数据外的长度 */
  minimum = sizeof(struct FrameHead) + sizeof(struct FrameTail);
  
  /* buffer中的数据有足够的数量 */
  if (read_bytes >= minimum)
  {
    /* index <= (read_bytes - minimum):确保索引不会超出范围 */
    for (index = 0; index <= (read_bytes - minimum); index++)
    {
      head = (struct FrameHead *)&tmp_buffer[index];		  
      
      if ( (FRAME_BEGIN == FRAME_NTOHS(head->begin)) && 
          (FRAME_NTOHS(head->length) <= (read_bytes - minimum)) )
      {
        tail = (struct FrameTail *)&tmp_buffer[index + sizeof(struct FrameHead) + head->length];
        
        if (FRAME_END == FRAME_NTOHS(tail->end))
        {									
          /* 数据帧长度 */
          length = minimum + FRAME_NTOHS(head->length);
          /* 接收数据拷贝的buffer过小 */
          if (length > size) goto __label_out;
          
          memcpy(buffer, head, length);
          find_frame = true;
          
          break;
        }
      }
    }
  }
  /* frame_rx_buffer中数据过短 */
  else
    goto __label_out;
   
  /* 缓存中有完整的数据帧，将数据拷贝出来 */
  if (find_frame)
  {
    /* 仅仅只是拷贝出来，腾出frame_rx_buffer空间 */
    ringbuffer_get_bytes(FrameRbuffer, tmp_buffer, length + index);
  }

__label_out:  
  if (tmp_buffer != NULL) free(tmp_buffer);
  
  return length;
}