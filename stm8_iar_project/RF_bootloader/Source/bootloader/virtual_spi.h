#ifndef __VIRTUAL_SPI_H__
#define __VIRTUAL_SPI_H__
/*
 *******************************************************************************
 *                                  TYPEDEFS                                  
 *******************************************************************************
 */
struct virtual_spi_bus
{
  void (* scl_set)(unsigned char  bit);
  void (* mosi_set)(unsigned char bit);
  unsigned char (* miso_get)(void);
};

struct virtual_spi_device
{
  struct virtual_spi_bus *bus;
  void (* cs_set)(unsigned char bit);
  void (* delay)(void);
};

/*
 *******************************************************************************
 *                             PUBLIC VARIABLES                                  
 *******************************************************************************
 */
extern struct virtual_spi_bus spi_bus_1;

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void spi_init(void);

void spi_device_select(struct virtual_spi_device *spi_device);

void spi_device_deselect(struct virtual_spi_device *spi_device);

unsigned char spi_send_recv(struct virtual_spi_device *spi_device, unsigned char data);

unsigned short spi_send_recv_16(struct virtual_spi_device *spi_device, unsigned short data);
#endif

