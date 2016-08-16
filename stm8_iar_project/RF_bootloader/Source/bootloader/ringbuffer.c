/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "ringbuffer.h"
#include <stdlib.h>
#include <string.h>

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
*/
ringbuffer_t *ringbuffer_create(uint32_t size)
{
  struct ringbuffer *rb;
  
  
  if (0 == size) return NULL;
  
  rb = (struct ringbuffer *)malloc(sizeof(struct ringbuffer));
  if (NULL == rb) return NULL;
  
  rb->buffer = (uint8_t *)malloc(size);
  if (NULL == rb->buffer)
  {
    free(rb);
    return NULL;
  }
  
  rb->write_index = 0;
  rb->read_index  = 0;
  rb->buffer_size = size;
  
  return rb;
}

void ringbuffer_destroy(ringbuffer_t *rb)
{
  if (NULL == rb) return;
  
  if (rb->buffer != NULL) free(rb->buffer);
  
  free(rb);
}

uint32_t ringbuffer_put_bytes(ringbuffer_t *rb, uint8_t *data, uint32_t length)
{
  uint32_t size;
  
  
  if ( (NULL == rb) || (NULL == data) || (0 == length) ) return 0;
  
  /* �ж��Ƿ����㹻��ʣ��ռ� */
  if (rb->read_index > rb->write_index)
    size = rb->read_index - rb->write_index;
  else
    size = rb->buffer_size - rb->write_index + rb->read_index;
  
  /* û�ж���Ŀռ� */
  if (size == 0) 
  {
    return 0;
  }
  
  /* ���ݲ����������������ݣ��ضϷ��� */
  if (size < length) length = size;
  
  if (rb->read_index > rb->write_index)
  {
    /* rb->read_index - rb->write_index ��Ϊ�ܵĿ���ռ� */
    memcpy(&rb->buffer[rb->write_index], data, length);
    rb->write_index += length;
  }
  else
  {
    if (rb->buffer_size - rb->write_index > length)
    {
      /* rb->write_index ����ʣ��Ŀռ����㹻�ĳ��� */
      memcpy(&rb->buffer[rb->write_index], data, length);
      rb->write_index += length;
    }
    else
    {
      /*
      * rb->write_index ����ʣ��Ŀռ䲻�����㹻�ĳ��ȣ���Ҫ�Ѳ������ݸ��Ƶ�
      * ǰ���ʣ��ռ���
      */
      memcpy(&rb->buffer[rb->write_index], data, rb->buffer_size - rb->write_index);
      memcpy(&rb->buffer[0], &data[rb->buffer_size - rb->write_index],  length - (rb->buffer_size - rb->write_index));
      rb->write_index = length - (rb->buffer_size - rb->write_index);
    }
  }
  
  return length;
}

uint32_t ringbuffer_put_one_byte(ringbuffer_t *rb, uint8_t value)
{
  uint32_t next;
  
  if (NULL == rb) return 0;
  
  /* �ж��Ƿ��ж���Ŀռ� */
  next = rb->write_index + 1;
  if (next >= rb->buffer_size) next = 0;
  
  if (next == rb->read_index) 
  {
    return 0;
  }
  
  /* �����ַ� */
  rb->buffer[rb->write_index] = value;
  rb->write_index = next;
  
  return 1ul;
}

uint32_t ringbuffer_get_bytes(ringbuffer_t *rb, uint8_t *buffer, uint32_t length)
{
  uint32_t size;
  
  
  if ( (NULL == rb) || (NULL == buffer) || (0 == length) ) return 0;
  
  /* �ж��Ƿ����㹻������ */
  if (rb->read_index > rb->write_index)
    size = rb->buffer_size - rb->read_index + rb->write_index;
  else
    size = rb->write_index - rb->read_index;
  
  /* û���㹻������ */
  if (size == 0)
  {
    return 0;
  }
  
  /* ���ݲ���ָ���ĳ��ȣ�ȡ����buffer��ʵ�ʵĳ��� */
  if (size < length) length = size;
  
  if (rb->read_index > rb->write_index)
  {
    if (rb->buffer_size - rb->read_index > length)
    {
      /* rb->read_index�������㹻�ֱ࣬�Ӹ��� */
      memcpy(buffer, &rb->buffer[rb->read_index], length);
      rb->read_index += length;
    }
    else
    {
      /* rb->read_index�����ݲ�������Ҫ�ֶθ��� */
      memcpy(buffer, &rb->buffer[rb->read_index], rb->buffer_size - rb->read_index);
      memcpy(&buffer[rb->buffer_size - rb->read_index], &rb->buffer[0], length - rb->buffer_size + rb->read_index);
      rb->read_index = length - rb->buffer_size + rb->read_index;
    }
  }
  else
  {
    /*
    * rb->read_indexҪ��rb->write_indexС���ܵ�����������ǰ���Ѿ���������������
    * �ϣ���ֱ�Ӹ��Ƴ����ݡ�
    */
    memcpy(buffer, &rb->buffer[rb->read_index], length);
    rb->read_index += length;
  }

  return length;
}

uint32_t ringbuffer_get_one_byte(ringbuffer_t *rb, uint8_t *value)
{
  if ( (NULL == rb) || (NULL == value) ) return 0;
  
  /* �յ�Buffer */
  if (rb->read_index == rb->write_index) 
  {
    return 0;
  }
  
  *value = rb->buffer[rb->read_index];
  
  rb->read_index = (rb->read_index + 1) % rb->buffer_size;
  
  return 1;
} 

uint32_t ringbuffer_view_bytes(ringbuffer_t *rb, uint8_t *buffer, uint32_t size)
{
  uint32_t i, count;
  
  
  if ( (NULL == rb) || (NULL == buffer) || (0 == size) ) return 0;
  
  /* �յ�Buffer */
  if (rb->read_index == rb->write_index) 
  {
    return 0;
  }
  
  for  (i = rb->read_index, count = 0; 
       (i != rb->write_index) && (count < size); 
       i = (i + 1) % rb->buffer_size, count++)
  {
    buffer[count] = rb->buffer[i];
  }

  return count;
}

uint32_t ringbuffer_data_size(ringbuffer_t *rb)
{
  uint32_t size = 0;
  
  
  if (rb != NULL)
  {  
    /* �յ�Buffer */
    if (rb->read_index == rb->write_index) 
    {
      size = 0;
    }
    else
    {
      if (rb->read_index > rb->write_index)
        size = rb->buffer_size - rb->read_index + rb->write_index;
      else
        size = rb->write_index - rb->read_index;
    }
  }
  
  return size;
}


