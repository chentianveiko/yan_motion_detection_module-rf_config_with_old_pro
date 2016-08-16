#ifndef RING_BUFFER_H
#define RING_BUFFER_H
/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "port.h"

/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */

struct ringbuffer
{
  uint8_t   *buffer;
  uint32_t   buffer_size;
  uint32_t   write_index;
  uint32_t   read_index;
};	

typedef struct ringbuffer ringbuffer_t;

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
*/
ringbuffer_t *ringbuffer_create(uint32_t size);

void ringbuffer_destroy(ringbuffer_t *rb);

uint32_t ringbuffer_put_bytes(ringbuffer_t *rb, uint8_t *data, uint32_t length);

uint32_t ringbuffer_put_one_byte(ringbuffer_t *rb, uint8_t value);

uint32_t ringbuffer_get_bytes(ringbuffer_t *rb, uint8_t *buffer, uint32_t length);

uint32_t ringbuffer_get_one_byte(ringbuffer_t *rb, uint8_t *value); 

uint32_t ringbuffer_view_bytes(ringbuffer_t *rb, uint8_t *buffer, uint32_t size);

uint32_t ringbuffer_data_size(ringbuffer_t *rb);

#endif 

