/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "hal_rf.h"
#include "virtual_spi.h"
#include "stm8l15x.h"
#include <string.h>
#include "stdbool.h"

#if (defined CENTER)
#include "hal_led.h"
#endif

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#define A7139_1_SPI_CS_GPIO         GPIOB
#define A7139_1_SPI_CS_PIN          GPIO_Pin_4
#define A7139_1_WTR_GPIO            GPIOB
#define A7139_1_WTR_PIN             GPIO_Pin_3
#define A7139_1_EXTI_PIN            EXTI_Pin_3
#define A7139_1_EXTI_IT_PIN         EXTI_IT_Pin3
#define HAL_RF1_INTERRUPT_HANDLER() INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)

/* A7139_ Defines */
#define SYSTEMCLOCK_REG 	0x00
#define PLL1_REG 		0x01
#define PLL2_REG 		0x02
#define PLL3_REG 		0x03
#define PLL4_REG		0x04
#define PLL5_REG		0x05
#define PLL6_REG		0x06
#define CRYSTAL_REG		0x07
#define PAGEA_REG	  	0x08
#define PAGEB_REG		0x09
#define RX1_REG  		0x0A
#define RX2_REG  		0x0B
#define ADC_REG  		0x0C
#define PIN_REG 		0x0D
#define CALIBRATION_REG  	0x0E
#define MODE_REG  		0x0F

#define TX1_PAGEA               0x00
#define WOR1_PAGEA              0x01
#define WOR2_PAGEA              0x02
#define RFI_PAGEA               0x03
#define PM_PAGEA                0x04
#define RTH_PAGEA               0x05
#define AGC1_PAGEA              0x06
#define AGC2_PAGEA              0x07
#define GIO_PAGEA               0x08
#define CKO_PAGEA               0x09
#define VCB_PAGEA               0x0A
#define CHG1_PAGEA              0x0B
#define CHG2_PAGEA              0x0C
#define FIFO_PAGEA	       	0x0D
#define CODE_PAGEA		0x0E
#define WCAL_PAGEA		0x0F

#define TX2_PAGEB		0x00
#define	IF1_PAGEB		0x01
#define IF2_PAGEB		0x02
#define	ACK_PAGEB		0x03
#define	ART_PAGEB		0x04

#define CMD_Reg_W		0x00	//000x,xxxx	control register write
#define CMD_Reg_R		0x80	//100x,xxxx	control register read
#define CMD_ID_W		0x20	//001x,xxxx	ID write
#define CMD_ID_R		0xA0	//101x,xxxx	ID Read
#define CMD_FIFO_W		0x40	//010x,xxxx	TX FIFO Write
#define CMD_FIFO_R		0xC0	//110x,xxxx	RX FIFO Read
#define CMD_RF_RST		0x70	//x111,xxxx	RF reset
#define CMD_TFR			0x60	//0110,xxxx	TX FIFO address pointrt reset
#define CMD_RFR			0xE0	//1110,xxxx	RX FIFO address pointer reset

#define CMD_SLEEP		0x10	//0001,0000	SLEEP mode
#define CMD_IDLE		0x12	//0001,0010	IDLE mode
#define CMD_STBY		0x14	//0001,0100	Standby mode
#define CMD_PLL			0x16	//0001,0110	PLL mode
#define CMD_RX			0x18	//0001,1000	RX mode
#define CMD_TX			0x1A	//0001,1010	TX mode
//#define CMD_DEEP_SLEEP	0x1C	//0001,1100 Deep Sleep mode(tri-state)
#define CMD_DEEP_SLEEP		0x1F	//0001,1111 Deep Sleep mode(pull-high)

/* 定义FIFO大小 */
#define FIFO_R       0x003F

#if (HAL_RF_DATE_RATE == HAL_RF_DATE_RATE_0_5KBPS)
/* A7139 433.201M DR0.5kbps IFBW50kHz Fdev18.75kHz@12.8M -123dBm */
const uint16_t A7139Config[]=
{
  0xC623, /* SYSTEM CLOCK register,   */
  0x0A21, /* PLL1 register,           */
  0xDA05, /* PLL2 register,	      */
  0x0000, /* PLL3 register,           */
  0x0A20, /* PLL4 register,           */
  0x0024, /* PLL5 register,           */
  0x0000, /* PLL6 register,           */
  0x0011, /* CRYSTAL register,        */
  0x0000, /* PAGEA,                   */
  0x0000, /* PAGEB,                   */
  0x18D0, /* RX1 register, 	      */
  0x7009, /* RX2 register, 	      */
  0x4000, /* ADC register,	      */
  0x0800, /* PIN CONTROL register,    */
  0x4C45, /* CALIBRATION register,    */
  0x20C0, /* MODE CONTROL register,   */
};

const uint16_t A7139Config_PageA[]=
{
  0xF606, /* TX1 register,  */
  0x0000, /* WOR1 register, */
  0xF800, /* WOR2 register, */
  0x1107, /* RFI register,  */
  0x8970, /* PM register,   */
  0x0201, /* RTH register,  */
  0x400F, /* AGC1 register, */
  0x2AC0, /* AGC2 register, */
  0x0041, /* GIO register,  */
  0xD181, /* CKO register   */
  0x0004, /* VCB register,  */
  0x0A21, /* CHG1 register, */
  0x0022, /* CHG2 register, */
  FIFO_R, /* FIFO register, */
  0x1517, /* CODE register, */
  0x0000, /* WCAL register, */
};

const uint16_t A7139Config_PageB[]=
{
  0x0337, /* TX2 register */
  0x8200, /* IF1 register */
  0x0000, /* IF2 register */
  0x0000, /* ACK register */
  0x0000, /* ART register */
};
#endif

#if (HAL_RF_DATE_RATE == HAL_RF_DATE_RATE_1_5KBPS)
/* 433M 1.5Kbps Fdev=18.75kHz IFBW=48KHz Xtal12.8MHz */
const uint16_t A7139Config[]=
{
  0x3EC7, /* SYSTEM CLOCK register,   */
  0x0A21, /* PLL1 register,           */
  0xDA05, /* PLL2 register,	      */
  0x0000, /* PLL3 register,           */
  0x0A20, /* PLL4 register,           */
  0x0024, /* PLL5 register,           */
  0x0000, /* PLL6 register,           */
  0x0003, /* CRYSTAL register,        */
  0x0000, /* PAGEA,                   */
  0x0000, /* PAGEB,                   */
  0x18D0, /* RX1 register, 	      */
  0x7009, /* RX2 register, 	      */
  0x4400, /* ADC register,	      */
  0x0800, /* PIN CONTROL register,    */
  0x4C45, /* CALIBRATION register,    */
  0x20C0, /* MODE CONTROL register,   */
};

const uint16_t A7139Config_PageA[]=
{
  0xF606, /* TX1 register,  */
  0x0000, /* WOR1 register, */
  0xF800, /* WOR2 register, */
  0x1107, /* RFI register,  */
  0x0170, /* PM register,   */
  0x0201, /* RTH register,  */
  0x400F, /* AGC1 register, */
  0x2AC0, /* AGC2 register, */
  0xC041, /* GIO register,  */
  0xD283, /* CKO register   */
  0x0004, /* VCB register,  */
  0x0A21, /* CHG1 register, */
  0x0022, /* CHG2 register, */
  FIFO_R, /* FIFO register, */
  0x1517, /* CODE register, */
  0x0000, /* WCAL register, */
};

const uint16_t A7139Config_PageB[]=
{
  0x837F, /* TX2 register */
  0x81EC, /* IF1 register */
  0x0000, /* IF2 register */
  0x0000, /* ACK register */
  0x0000, /* ART register */
};
#endif

#if (HAL_RF_DATE_RATE == HAL_RF_DATE_RATE_10KBPS)
/* 433MHz, 10kbps (IFBW = 100KHz, Fdev = 37.5KHz), Crystal=12.8MHz */
const uint16_t A7139Config[]=
{
  0x1221, /* SYSTEM CLOCK register,   */
  0x0A21, /* PLL1 register,           */
  0xDA05, /* PLL2 register,	      */
  0x0000, /* PLL3 register,           */
  0x0A20, /* PLL4 register,           */
  0x0024, /* PLL5 register,           */
  0x0000, /* PLL6 register,           */
  0x0011, /* CRYSTAL register,        */
  0x0000, /* PAGEA,                   */
  0x0000, /* PAGEB,                   */
  0x18D4, /* RX1 register, 	      */
  0x7009, /* RX2 register, 	      */
  0x4000, /* ADC register,	      */
  0x0800, /* PIN CONTROL register,    */
  0x4C45, /* CALIBRATION register,    */
  0x20C0, /* MODE CONTROL register,   */
};

const uint16_t A7139Config_PageA[]=
{
  0xF706, /* TX1 register,   */
  0x0000, /* WOR1 register,  */
  0xF800, /* WOR2 register,  */
  0x1107, /* RFI register,   */
  0x8170, /* PM register,    */
  0x0201, /* RTH register,   */
  0x400F, /* AGC1 register,  */
  0x2AC0, /* AGC2 register,  */
  0x0045, /* GIO register,   */
  0xD181, /* CKO register    */
  0x0004, /* VCB register,   */
  0x0A21, /* CHG1 register,  */
  0x0022, /* CHG2 register,  */
  FIFO_R, /* FIFO register,  */
  0x1517, /* CODE register,  */
  0x0000, /* WCAL register,  */
};

const uint16_t A7139Config_PageB[]=
{
  0x037F, /* TX2 register, */
  0x8400, /* IF1 register, */
  0x0000, /* IF2 register, */
  0x0000, /* ACK register, */
  0x0000, /* ART register, */
};
#endif

const uint16_t A7139ChannelValues[HAL_RF_CHANNEL_NB] =
{
  0xDA05, /* 433MHz */
  0xB205, /* 431MHz */
  0x8A05, /* 429MHz */
  0x6205, /* 427MHz */
  0x3A05, /* 425MHz */
  0x1205, /* 423MHz */
};

/*
 *******************************************************************************
 *                                  TYPEDEFS
 *******************************************************************************
 */
enum DeviceState {
	Init, Standby, Sending, SendDone, Receiving, ReceiveDone,
};

/*
 *******************************************************************************
 *                                  MACROS
 *******************************************************************************
 */
#define HAL_RF_ID_TO_OBJECT(id)  ((struct A7139_Device *)(((id) >= HalRFDeviceNum) ? NULL : &A7139_DeviceList[(id)]))

/*
 *******************************************************************************
 *                                  TYPEDEFS
 *******************************************************************************
 */
struct A7139_Device {
	enum HalRFDevice device;
	struct virtual_spi_device *spi_device;
	enum DeviceState state;
	int rssi;
};

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
#if (defined HAL_RF_1)
static void A7139_1_SpiSelect(uint8_t set);
#endif

#if (defined HAL_RF_2)
static void A7139_2_SpiSelect(uint8_t set);
#endif

static void A7139_SpiDelay(void);
static void GPIO_Configration(void);

static void A7139_Delay(uint32_t ms);

static void A7139_WriteCommand(struct A7139_Device *device, uint8_t cmd);
static void A7139_WriteID(struct A7139_Device *device, uint32_t id);
static void A7139_WriteRegister(struct A7139_Device *device, uint8_t address, uint16_t data);
static uint16_t A7139_ReadRegister(struct A7139_Device *device, uint8_t address);
static void A7139_WritePageA(struct A7139_Device *device, uint8_t address, uint16_t dataWord);
static uint16_t A7139_ReadPageA(struct A7139_Device *device, uint8_t address);
static void A7139_WritePageB(struct A7139_Device *device, uint8_t address, uint16_t dataWord);
static uint16_t A7139_ReadPageB(struct A7139_Device *device, uint8_t address);
static void A7139_Config(struct A7139_Device *device);
static void A7139_Calibration(struct A7139_Device *device);
static void A7139_WriteFIFO(struct A7139_Device *device, uint8_t *data, uint8_t length);
static uint8_t A7139_ReadFIFO(struct A7139_Device *device, uint8_t *data, uint8_t size);
static void A7139_StartRSSIMeasurement(struct A7139_Device *device);

/*
 *******************************************************************************
 *                              LOCAL VARIABLES
 *******************************************************************************
 */
#if (defined HAL_RF_1)
struct virtual_spi_device A7139_SPIDevice1 =
{
	.bus = &spi_bus_1,
	.cs_set = A7139_1_SpiSelect,
	.delay = A7139_SpiDelay,
};
#endif

#if (defined HAL_RF_2)
struct virtual_spi_device A7139_SPIDevice2 =
{
	.bus = &spi_bus_1,
	.cs_set = A7139_2_SpiSelect,
	.delay = A7139_SpiDelay,
};
#endif

struct A7139_Device A7139_DeviceList[] = {
#if (defined HAL_RF_1)
		{	.device = HalRF1, .spi_device = &A7139_SPIDevice1, .state = Init,},
#endif

#if (defined HAL_RF_2)
		{	.device = HalRF1, .spi_device = &A7139_SPIDevice2, .state = Init,},
#endif
	};

/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
void HalRFInit(void) {
	struct A7139_Device *device;
	int i;
	uint32_t id = 0x11223344;

	GPIO_Configration();

	for (i = 0; i < HalRFDeviceNum; i++) {
		device = &A7139_DeviceList[i];

		if (NULL == device)
			continue;

		device->state = Init;

		A7139_WriteCommand(device, CMD_RF_RST);
		A7139_Delay(1);
		A7139_Config(device);
		A7139_Delay(1);
		A7139_WriteID(device, id);
		A7139_Calibration(device);
		A7139_Delay(1);
	}
}

void HalRFSend(enum HalRFDevice rf, unsigned char *data, unsigned char length) {
	struct A7139_Device *device;

	device = HAL_RF_ID_TO_OBJECT(rf);

	if (NULL == device)
		return;

	device->state = Standby;
	A7139_WriteCommand(device, CMD_STBY);

	device->state = Sending;
	A7139_WriteFIFO(device, data, length);
	A7139_WriteCommand(device, CMD_TX);
}

unsigned char HalRFReceive(enum HalRFDevice rf, unsigned char *data, unsigned char size) {
	struct A7139_Device *device;
	unsigned char length;

	device = HAL_RF_ID_TO_OBJECT(rf);

	if (NULL == device)
		return 0;

	length = A7139_ReadFIFO(device, data, size);

	A7139_StartRSSIMeasurement(device);
	device->state = Receiving;
	A7139_WriteCommand(device, CMD_RX);

#if (defined CENTER)
	HalLedBlink(HAL_LED_RF, 50, 20, 1);
#endif

	return length;
}

int HalRFReadRSSI(enum HalRFDevice rf) {
	struct A7139_Device *device;
	int value;

	device = HAL_RF_ID_TO_OBJECT(rf);

	value = device->rssi;

	return value;
}

bool HalRFSetChannel(enum HalRFDevice rf, HalRFChannel_t ch) {
	struct A7139_Device *device;
	bool result = FALSE;
	int index;

	device = HAL_RF_ID_TO_OBJECT(rf);

	if ((device != NULL) && (ch < HAL_RF_CHANNEL_NB)) {
		index = ch;
		A7139_WriteRegister(device, PLL2_REG, A7139ChannelValues[index]);
		result = TRUE;
	}

	return result;
}

#define DEEP_SLEEP 1
void HalRFEnterSleep(enum HalRFDevice rf) {
	struct A7139_Device *device;

	device = HAL_RF_ID_TO_OBJECT(rf);

#if DEEP_SLEEP
	A7139_WriteCommand(device, CMD_DEEP_SLEEP);
#else
	A7139_WriteCommand(device, CMD_SLEEP);
#endif

	GPIO_Init(A7139_1_SPI_CS_GPIO, A7139_1_SPI_CS_PIN, GPIO_Mode_Out_PP_Low_Slow);
	GPIO_Init(A7139_1_WTR_GPIO, A7139_1_WTR_PIN, GPIO_Mode_Out_PP_Low_Slow);
}

void HalRFExitSleep(enum HalRFDevice rf) {
	struct A7139_Device *device;
	uint8_t i;
	uint32_t id = 0x11223344;

	device = HAL_RF_ID_TO_OBJECT(rf);

	GPIO_Configration();
	disableInterrupts();
	A7139_WriteCommand(device, CMD_STBY);

#if DEEP_SLEEP
	for (i = 0; i < HalRFDeviceNum; i++) {
		device = &A7139_DeviceList[i];

		if (NULL == device)
			continue;

		device->state = Init;

		A7139_WriteCommand(device, CMD_RF_RST);
		A7139_Delay(1);
		A7139_Config(device);
		A7139_Delay(1);
		A7139_WriteID(device, id);
		A7139_Calibration(device);
		A7139_Delay(1);
	}
#endif
	enableInterrupts();
}

/*
 *******************************************************************************
 *                              LOCAL FUNCTIONS
 *******************************************************************************
 */
#if (defined HAL_RF_1)
static void A7139_1_SpiSelect(uint8_t set)
{
	GPIO_WriteBit(A7139_1_SPI_CS_GPIO, A7139_1_SPI_CS_PIN, (BitAction)set);
}
#endif

#if (defined HAL_RF_2)
static void A7139_2_SpiSelect(uint8_t set)
{
	GPIO_WriteBit(A7139_2_SPI_CS_GPIO, A7139_2_SPI_CS_PIN, (BitAction)set);
}
#endif

static void A7139_SpiDelay(void) {
	;
}

static void GPIO_Configration(void) {
	/* A7139 Device1 CS Pin */
	GPIO_Init(A7139_1_SPI_CS_GPIO, A7139_1_SPI_CS_PIN, GPIO_Mode_Out_PP_High_Fast);
	GPIO_WriteBit(A7139_1_SPI_CS_GPIO, A7139_1_SPI_CS_PIN, SET);

	/* A7139 Device1 GIO1 Pin */
	GPIO_Init(A7139_1_WTR_GPIO, A7139_1_WTR_PIN, GPIO_Mode_In_PU_IT);
	EXTI_SetPinSensitivity(A7139_1_EXTI_PIN, EXTI_Trigger_Falling);
}

static void A7139_Delay(uint32_t ms) {
	uint32_t i = 0;

	/* Enable TIM2 Clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);

	/* Configure TIM2 to generate an update event each 1 s */
	TIM2_TimeBaseInit(TIM2_Prescaler_128, TIM2_CounterMode_Up, 125);

	/* Enable TIM2 */
	TIM2_Cmd (ENABLE);

	/* Clear the Flag */
	TIM2_ClearFlag (TIM2_FLAG_Update);
	TIM2_ARRPreloadConfig(ENABLE);

	for (i = 0; i < ms; i++) {
		/* Wait 1 ms */
		while (TIM2_GetFlagStatus(TIM2_FLAG_Update) == RESET) {
		}

		/* Clear the Flag */
		TIM2_ClearFlag(TIM2_FLAG_Update);
	}

	/* Disable TIM2 */
	TIM2_Cmd (DISABLE);

	/* Disable TIM2 Clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, DISABLE);
}

/*
 *******************************************************************************
 @brief    向A7139写入命令

 @params   device - A7139设备对象
 cmd - 写入的命令

 @return   无
 *******************************************************************************
 */
static void A7139_WriteCommand(struct A7139_Device *device, uint8_t cmd) {
	struct virtual_spi_device *spi_device;

	if (device != NULL) {
		spi_device = device->spi_device;

		spi_device_select(spi_device);
		spi_send_recv(spi_device, cmd);
		spi_device_deselect(spi_device);
	}
}

/*
 *******************************************************************************
 @brief    向A7139写入ID

 @params   device - A7139设备对象
 id - 写入的id

 @return   无
 *******************************************************************************
 */
static void A7139_WriteID(struct A7139_Device *device, uint32_t id) {
	volatile uint8_t read_id[4] = { 0 };
	int i;
	struct virtual_spi_device *spi_device;

	if (device != NULL) {
		spi_device = device->spi_device;

		/* 写入ID */
		spi_device_select(spi_device);

		spi_send_recv(spi_device, CMD_ID_W);

		spi_send_recv(spi_device, (uint8_t)(id >> 24));
		spi_send_recv(spi_device, (uint8_t)(id >> 16));
		spi_send_recv(spi_device, (uint8_t)(id >> 8));
		spi_send_recv(spi_device, (uint8_t)(id >> 0));

		spi_device_deselect(spi_device);

		/* 读取ID验证通信是否正常 */
		spi_device_select(spi_device);

		spi_send_recv(spi_device, CMD_ID_R);

		for (i = 0; i < 4; i++)
			read_id[i] = spi_send_recv(spi_device, 0);

		spi_device_deselect(spi_device);
	}
}

/*
 *******************************************************************************
 @brief    向A7139寄存器写入数据

 @params   device - A7139设备对象
 address - 寄存器地址
 data    - 写入豁口的数据

 @return   无
 *******************************************************************************
 */
static void A7139_WriteRegister(struct A7139_Device *device, uint8_t address, uint16_t data) {
	struct virtual_spi_device *spi_device;

	if (device != NULL) {
		spi_device = device->spi_device;

		spi_device_select(spi_device);

		spi_send_recv(spi_device, (address | CMD_Reg_W));
		spi_send_recv_16(spi_device, data);

		spi_device_deselect(spi_device);
	}
}

/*
 *******************************************************************************
 @brief    从A7139寄存器读出数据

 @params   device - A7139设备对象
 address - 寄存器地址

 @return   无
 *******************************************************************************
 */
static uint16_t A7139_ReadRegister(struct A7139_Device *device, uint8_t address) {
	struct virtual_spi_device *spi_device;
	uint16_t read_data = 0;

	if (device != NULL) {
		spi_device = device->spi_device;

		spi_device_select(spi_device);

		spi_send_recv(spi_device, (address | CMD_Reg_R));
		read_data = spi_send_recv_16(spi_device, 0);

		spi_device_deselect(spi_device);
	}

	return read_data;
}

/*
 *******************************************************************************
 @brief    A7139写PAGEA

 @params   device - A7139设备对象
 address -
 dataWord -

 @return   无
 *******************************************************************************
 */
static void A7139_WritePageA(struct A7139_Device *device, uint8_t address, uint16_t dataWord) {
	uint16_t tmp;

	tmp = address;
	tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);

	A7139_WriteRegister(device, CRYSTAL_REG, tmp);
	A7139_WriteRegister(device, PAGEA_REG, dataWord);
}

static uint16_t A7139_ReadPageA(struct A7139_Device *device, uint8_t address) {
	uint16_t tmp;

	tmp = address;
	tmp = ((tmp << 12) | A7139Config[CRYSTAL_REG]);
	A7139_WriteRegister(device, CRYSTAL_REG, tmp);
	tmp = A7139_ReadRegister(device, PAGEA_REG);

	return tmp;
}

/*
 *******************************************************************************
 @brief    A7139写PAGEB

 @params   device - A7139设备对象
 address -
 dataWord -

 @return   无
 *******************************************************************************
 */
static void A7139_WritePageB(struct A7139_Device *device, uint8_t address, uint16_t dataWord) {
	uint16_t tmp;

	tmp = address;
	tmp = ((tmp << 7) | A7139Config[CRYSTAL_REG]);

	A7139_WriteRegister(device, CRYSTAL_REG, tmp);
	A7139_WriteRegister(device, PAGEB_REG, dataWord);
}

static uint16_t A7139_ReadPageB(struct A7139_Device *device, uint8_t address) {
	uint16_t tmp;

	tmp = address;
	tmp = ((tmp << 7) | A7139Config[CRYSTAL_REG]);
	A7139_WriteRegister(device, CRYSTAL_REG, tmp);
	tmp = A7139_ReadRegister(device, PAGEB_REG);

	return tmp;
}

/*
 *******************************************************************************
 @brief    向A7139写入配置序列表

 @params   device - A7139设备对象

 @return   无
 *******************************************************************************
 */
static void A7139_Config(struct A7139_Device *device) {
	uint8_t i;

	for (i = 0; i < 8; i++)
		A7139_WriteRegister(device, i, A7139Config[i]);

	for (i = 10; i < 16; i++)
		A7139_WriteRegister(device, i, A7139Config[i]);

	for (i = 0; i < 16; i++)
		A7139_WritePageA(device, i, A7139Config_PageA[i]);

	for (i = 0; i < 5; i++)
		A7139_WritePageB(device, i, A7139Config_PageB[i]);
}

/*
 *******************************************************************************
 @brief    IF and VCO calibration

 @params   device - A7139设备对象

 @return   无
 *******************************************************************************
 */
static void A7139_Calibration(struct A7139_Device *device) {
	volatile uint8_t fb, fcd, fbcf;	//IF Filter
	volatile uint8_t vb, vbcf;              //VCO Current
	volatile uint8_t vcb, vccf;		//VCO Band
	uint16_t tmp;

	//IF calibration procedure @STB state
	A7139_WriteRegister(device, MODE_REG, A7139Config[MODE_REG] | 0x0802);			//IF Filter & VCO Current Calibration
	do {
		tmp = A7139_ReadRegister(device, MODE_REG);
	} while (tmp & 0x0802);

	//for check(IF Filter)
	tmp = A7139_ReadRegister(device, CALIBRATION_REG);
	fb = tmp & 0x0F;
	fcd = (tmp >> 11) & 0x1F;
	fbcf = (tmp >> 4) & 0x01;
	if (fbcf) {

	}

	//for check(VCO Current)
	tmp = A7139_ReadPageA(device, VCB_PAGEA);
	vcb = tmp & 0x0F;
	vccf = (tmp >> 4) & 0x01;
	if (vccf) {

	}

	//RSSI Calibration procedure @STB state
	A7139_WriteRegister(device, ADC_REG, 0x4C00);									//set ADC average=64
	A7139_WriteRegister(device, MODE_REG, A7139Config[MODE_REG] | 0x1000);			//RSSI Calibration
	do {
		tmp = A7139_ReadRegister(device, MODE_REG);
	} while (tmp & 0x1000);
	A7139_WriteRegister(device, ADC_REG, A7139Config[ADC_REG]);

	//VCO calibration procedure @STB state
	A7139_WriteRegister(device, PLL1_REG, A7139Config[PLL1_REG]);
	A7139_WriteRegister(device, PLL2_REG, A7139Config[PLL2_REG]);
	A7139_WriteRegister(device, MODE_REG, A7139Config[MODE_REG] | 0x0004);		//VCO Band Calibration
	do {
		tmp = A7139_ReadRegister(device, MODE_REG);
	} while (tmp & 0x0004);

	//for check(VCO Band)
	tmp = A7139_ReadRegister(device, CALIBRATION_REG);
	vb = (tmp >> 5) & 0x07;
	vbcf = (tmp >> 8) & 0x01;
	if (vbcf) {

	}
}

static void A7139_WriteFIFO(struct A7139_Device *device, uint8_t *data, uint8_t length) {
	uint8_t i, max;
	struct virtual_spi_device *spi_device;

	if ((device != NULL) && (data != NULL) && (length != 0)) {
		spi_device = device->spi_device;

		A7139_WriteCommand(device, CMD_TFR);		//TX FIFO address pointer reset

		spi_device_select(spi_device);

		spi_send_recv(spi_device, CMD_FIFO_W); //TX FIFO write command

		max = length <= 64 ? length : 64;

		for (i = 0; i < max; i++)
			spi_send_recv(spi_device, data[i]);

		spi_device_deselect(spi_device);
	}
}

static uint8_t A7139_ReadFIFO(struct A7139_Device *device, uint8_t *data, uint8_t size) {
	uint8_t i, max;
	struct virtual_spi_device *spi_device;

	if ((device != NULL) && (data != NULL) && (size != 0)) {
		spi_device = device->spi_device;

		A7139_WriteCommand(device, CMD_RFR);		//TX FIFO address pointer reset

		spi_device_select(spi_device);

		spi_send_recv(spi_device, CMD_FIFO_R); //TX FIFO write command

		max = size <= 64 ? size : 64;

		for (i = 0; i < max; i++)
			data[i] = spi_send_recv(spi_device, 0);

		spi_device_deselect(spi_device);
	}

	return max;
}

int Hal_RfTxComplete;

extern void RFRawDataInput(uint8_t *data, uint16_t length);

static void A7139_InterruptHandler(struct A7139_Device *device) {
	uint16_t regTmp;
    uint8_t datain[100];
    uint8_t length;

	switch (device->state) {
	case Init:
		A7139_StartRSSIMeasurement(device);
		device->state = Receiving;
		A7139_WriteCommand(device, CMD_RX);
		break;

	case Standby:
		break;

	case Sending:
		device->state = SendDone;
		A7139_StartRSSIMeasurement(device);
		device->state = Receiving;
		A7139_WriteCommand(device, CMD_RX);
		Hal_RfTxComplete = 1;
		break;

	case Receiving:
		device->state = ReceiveDone;
		/* 读取RSSI */
		do {
			regTmp = A7139_ReadRegister(device, MODE_REG);
		} while ((regTmp & (1 << 0)));

		device->rssi = A7139_ReadRegister(device, RX2_REG) & 0x1FF;

		length = HalRFReceive(HalRF1, datain, 100);
		RFRawDataInput(datain, length);
		break;
	}
}

static void A7139_StartRSSIMeasurement(struct A7139_Device *device) {
	device->state = Standby;
	A7139_WriteCommand(device, CMD_STBY);

	/* Set bit CDM=0 in ADC register */
	A7139_WriteRegister(device, ADC_REG, A7139Config[ADC_REG] | 0x8000);
	A7139_ReadRegister(device, MODE_REG);
	/* Set bit ADCM=1 in mode control register to start the RSSI measurement */
	A7139_WriteRegister(device, MODE_REG, A7139Config[MODE_REG] | 0x0001);
}

#if (defined HAL_RF_1)
HAL_RF1_INTERRUPT_HANDLER()
{
	EXTI_ClearITPendingBit(A7139_1_EXTI_IT_PIN);

	A7139_InterruptHandler(&A7139_DeviceList[0]);
}
#endif

#if (defined HAL_RF_2)
INTERRUPT_HANDLER(EXTI3_IRQHandler, 11)
{
	EXTI_ClearITPendingBit(EXTI_IT_Pin3);
	A7139_InterruptHandler(&A7139_DeviceList[1]);
}
#endif
