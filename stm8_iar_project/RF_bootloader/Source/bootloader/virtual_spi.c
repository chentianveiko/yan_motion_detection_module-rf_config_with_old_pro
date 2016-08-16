/*
 *******************************************************************************
 *                                  INCLUDE                                  
 *******************************************************************************
 */
#include "virtual_spi.h"
#include "stm8l15x.h"

/*
 *******************************************************************************
 *                                 CONSTANTS                                     
 *******************************************************************************
 */
#define SPI_BUS_1_SCL_GPIO      GPIOB
#define SPI_BUS_1_SCL_PIN       GPIO_Pin_5
#define SPI_BUS_1_MOSI_GPIO     GPIOB
#define SPI_BUS_1_MOSI_PIN      GPIO_Pin_6
#define SPI_BUS_1_MISO_GPIO     GPIOB
#define SPI_BUS_1_MISO_PIN      GPIO_Pin_6

/*
 *******************************************************************************
 *                             SPI BUS FUNCTIONS                                 
 *******************************************************************************
 */
static void spi_bus_1_scl_set(uint8_t bit);
static void spi_bus_1_mosi_set(uint8_t bit);
static uint8_t spi_bus_1_miso_get(void);

/*
 *******************************************************************************
 *                             PUBLIC VARIABLES                                  
 *******************************************************************************
 */
struct virtual_spi_bus spi_bus_1 = 
{
  .scl_set  = spi_bus_1_scl_set,
  .mosi_set = spi_bus_1_mosi_set,
  .miso_get = spi_bus_1_miso_get,
};

/*
 *******************************************************************************
 *                                 FUNCTIONS                                 
 *******************************************************************************
 */
void spi_init(void)
{
  /* Initialize Virtual SPI Bus 1 GPIO */
  GPIO_Init(SPI_BUS_1_SCL_GPIO, SPI_BUS_1_SCL_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(SPI_BUS_1_MOSI_GPIO, SPI_BUS_1_MOSI_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_Init(SPI_BUS_1_MISO_GPIO, SPI_BUS_1_MISO_PIN, GPIO_Mode_Out_PP_High_Fast);
}

void spi_device_select(struct virtual_spi_device *spi_device)
{
  spi_device->cs_set(0);
}

void spi_device_deselect(struct virtual_spi_device *spi_device)
{
  spi_device->cs_set(1);
}

unsigned char spi_send_recv(struct virtual_spi_device *spi_device, unsigned char data)
{
  short         i;
  unsigned char read = 0, bit;
  
  
  for (i = 0x80; i >= 0x01; i >>= 1)
  {
    bit = ((data & i) == i) ? 1 : 0;     
    
    spi_device->bus->mosi_set(bit);
    spi_device->delay();
    spi_device->bus->scl_set(1);
    spi_device->delay();
    
    if (spi_device->bus->miso_get() > 0) read |= i;
    
    spi_device->bus->scl_set(0);
    spi_device->delay();
  }
  
  return read;
}

unsigned short spi_send_recv_16(struct virtual_spi_device *spi_device, unsigned short data)
{
  long i;
  unsigned short read = 0;
  unsigned char bit;
  
  
  for (i = 0x8000; i >= 0x01; i >>= 1)
  {
    bit = ((data & i) == i) ? 1 : 0;     
    
    spi_device->bus->mosi_set(bit);
    spi_device->delay();
    spi_device->bus->scl_set(1);
    spi_device->delay();
    
    if (spi_device->bus->miso_get() > 0) read |= i;
    
    spi_device->bus->scl_set(0);
    spi_device->delay();
  }
  
  return read;
}

/*
 *******************************************************************************
 *                             SPI BUS FUNCTIONS                                 
 *******************************************************************************
 */
static void spi_bus_1_scl_set(uint8_t bit)
{
  GPIO_WriteBit(SPI_BUS_1_SCL_GPIO, SPI_BUS_1_SCL_PIN, (BitAction)bit);
}

static void spi_bus_1_mosi_set(uint8_t bit)
{
  GPIO_Init(SPI_BUS_1_MOSI_GPIO, SPI_BUS_1_MOSI_PIN, GPIO_Mode_Out_PP_High_Fast);
  GPIO_WriteBit(SPI_BUS_1_MOSI_GPIO, SPI_BUS_1_MOSI_PIN, (BitAction)bit);
}

static uint8_t spi_bus_1_miso_get(void)
{
  GPIO_Init(SPI_BUS_1_MISO_GPIO, SPI_BUS_1_MISO_PIN, GPIO_Mode_In_PU_No_IT);
  return GPIO_ReadInputDataBit(SPI_BUS_1_MISO_GPIO, SPI_BUS_1_MISO_PIN);
}
