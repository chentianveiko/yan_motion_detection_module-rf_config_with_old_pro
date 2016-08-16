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
  
  /* 判断是否有足够的剩余空间 */
  if (rb->read_index > rb->write_index)
    size = rb->read_index - rb->write_index;
  else
    size = rb->buffer_size - rb->write_index + rb->read_index;
  
  /* 没有多余的空间 */
  if (size == 0) 
  {
    return 0;
  }
  
  /* 数据不够放置完整的数据，截断放入 */
  if (size < length) length = size;
  
  if (rb->read_index > rb->write_index)
  {
    /* rb->read_index - rb->write_index 即为总的空余空间 */
    memcpy(&rb->buffer[rb->write_index], data, length);
    rb->write_index += length;
  }
  else
  {
    if (rb->buffer_size - rb->write_index > length)
    {
      /* rb->write_index 后面剩余的空间有足够的长度 */
      memcpy(&rb->buffer[rb->write_index], data, length);
      rb->write_index += length;
    }
    else
    {
      /*
      * rb->write_index 后面剩余的空间不存在足够的长度，需要把部分数据复制到
      * 前面的剩余空间中
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
  
  /* 判断是否有多余的空间 */
  next = rb->write_index + 1;
  if (next >= rb->buffer_size) next = 0;
  
  if (next == rb->read_index) 
  {
    return 0;
  }
  
  /* 放入字符 */
  rb->buffer[rb->write_index] = value;
  rb->write_index = next;
  
  return 1ul;
}

uint32_t ringbuffer_get_bytes(ringbuffer_t *rb, uint8_t *buffer, uint32_t length)
{
  uint32_t size;
  
  
  if ( (NULL == rb) || (NULL == buffer) || (0 == length) ) return 0;
  
  /* 判断是否有足够的数据 */
  if (rb->read_index > rb->write_index)
    size = rb->buffer_size - rb->read_index + rb->write_index;
  else
    size = rb->write_index - rb->read_index;
  
  /* 没有足够的数据 */
  if (size == 0)
  {
    return 0;
  }
  
  /* 数据不够指定的长度，取环形buffer中实际的长度 */
  if (size < length) length = size;
  
  if (rb->read_index > rb->write_index)
  {
    if (rb->buffer_size - rb->read_index > length)
    {
      /* rb->read_index的数据足够多，直接复制 */
      memcpy(buffer, &rb->buffer[rb->read_index], length);
      rb->read_index += length;
    }
    else
    {
      /* rb->read_index的数据不够，需要分段复制 */
      memcpy(buffer, &rb->buffer[rb->read_index], rb->buffer_size - rb->read_index);
      memcpy(&buffer[rb->buffer_size - rb->read_index], &rb->buffer[0], length - rb->buffer_size + rb->read_index);
      rb->read_index = length - rb->buffer_size + rb->read_index;
    }
  }
  else
  {
    /*
    * rb->read_index要比rb->write_index小，总的数据量够（前面已经有总数据量的判
    * 断），直接复制出数据。
    */
    memcpy(buffer, &rb->buffer[rb->read_index], length);
    rb->read_index += length;
  }

  return length;
}

uint32_t ringbuffer_get_one_byte(ringbuffer_t *rb, uint8_t *value)
{
  if ( (NULL == rb) || (NULL == value) ) return 0;
  
  /* 空的Buffer */
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
  
  /* 空的Buffer */
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
    /* 空的Buffer */
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


