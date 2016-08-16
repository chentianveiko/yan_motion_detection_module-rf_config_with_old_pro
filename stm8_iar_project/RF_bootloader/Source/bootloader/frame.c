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
/* 8λ�������Ƕ��� */
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
  @brief    ��ʼ��ͨ�Ų�
 
  @params   ��
	
  @return   ��
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
  @brief    �������ݰ�
 
  @params   type  - ֡����
	    data  - ���͵ı��� 
	    length- ���ĳ���
	
  @return   ʵ�ʷ��͵��ֽ���������ʧ�ܷ���0
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
  @brief    ע�����ݵײ㷢�ͺ��������ջص�����
 
  @params   output  - �ײ㷢�ͺ���
            process - ���ջص�����
 
  @return   ��
 *******************************************************************************
 */
void FrameRegister(FrameOutput_t output, FrameProcess_t process)
{
  FrameOutputFunc      = output;
  FrameUserProcessFunc = process;
}

/*
 *******************************************************************************
  @brief    �����յ����ݰ�input����Frame���ڲ�����
 
  @params   data  - ���յı��� 
	    length- ���ĳ���
 
  @return   �����frame�ڲ�buffer�����ݳ��ȣ��������0��ʾframe�ڲ�buffer����
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
  @brief    ����ringbuffer�е�����
 
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
  @brief    CRC16У��������
 
  @params   buffer - ����У������ݰ�
            length - ���ݰ�����
 
  @return   CRC16У����
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
  @brief    �ӽ���buffer����ȡ��һ֡�������ݣ�����������ȷУ�飬ֻ��֤��������ȷ
	    �ĳ��ȼ�֡ͷ��֡β
 
  @params   buffer - ��������֡�Ļ���
            size   - ��������֡����ĳ���
 
  @return   �ɹ���ȡ��һ֡���ݷ�������֡�ĳ��ȣ����򷵻�0
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
  
  /* ���Ƴ�RingBuffer�����е�����,RingBuffer�е�������Ȼ���� */
  read_bytes = ringbuffer_view_bytes(FrameRbuffer, tmp_buffer, FRAME_BUFFER_SIZE);
  /* ����֡��С�ߴ磺����֡�г��û�������ĳ��� */
  minimum = sizeof(struct FrameHead) + sizeof(struct FrameTail);
  
  /* buffer�е��������㹻������ */
  if (read_bytes >= minimum)
  {
    /* index <= (read_bytes - minimum):ȷ���������ᳬ����Χ */
    for (index = 0; index <= (read_bytes - minimum); index++)
    {
      head = (struct FrameHead *)&tmp_buffer[index];		  
      
      if ( (FRAME_BEGIN == FRAME_NTOHS(head->begin)) && 
          (FRAME_NTOHS(head->length) <= (read_bytes - minimum)) )
      {
        tail = (struct FrameTail *)&tmp_buffer[index + sizeof(struct FrameHead) + head->length];
        
        if (FRAME_END == FRAME_NTOHS(tail->end))
        {									
          /* ����֡���� */
          length = minimum + FRAME_NTOHS(head->length);
          /* �������ݿ�����buffer��С */
          if (length > size) goto __label_out;
          
          memcpy(buffer, head, length);
          find_frame = true;
          
          break;
        }
      }
    }
  }
  /* frame_rx_buffer�����ݹ��� */
  else
    goto __label_out;
   
  /* ������������������֡�������ݿ������� */
  if (find_frame)
  {
    /* ����ֻ�ǿ����������ڳ�frame_rx_buffer�ռ� */
    ringbuffer_get_bytes(FrameRbuffer, tmp_buffer, length + index);
  }

__label_out:  
  if (tmp_buffer != NULL) free(tmp_buffer);
  
  return length;
}