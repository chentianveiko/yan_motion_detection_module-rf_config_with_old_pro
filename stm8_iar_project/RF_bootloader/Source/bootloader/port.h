#ifndef __PORT_H__
#define __PORT_H__
/*
 *******************************************************************************
 *                                  INCLUDE
 *******************************************************************************
 */
#include "stm8l15x.h"
#include "bootloader.h"
#include <stdlib.h>
#include "link.h"

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#ifndef NULL
#define NULL (void *)0
#endif

#ifndef false
#define false FALSE
#endif

#ifndef true
#define true  TRUE
#endif

/*
 *******************************************************************************
 *                                 CONSTANTS
 *******************************************************************************
 */
#define APP_BEGIN_ADDRESS   0xA800
#define APP_MAX_SIZE        (1024ul * 22ul)

/*
 *******************************************************************************
 *                                  MACROS
 *******************************************************************************
 */
#define port_malloc(n) malloc(n)
#define port_free(p)   free(p)

#define BL_HTONS(n)    (n)//(((n) & 0xFF) << 8) | (((n) & 0xFF00) >> 8)
#define BL_NTOHS(n)    BL_HTONS(n)
#define BL_HTONL(n)    (n)//(((n) & 0xFF) << 24) |(((n) & 0xFF00) << 8) | (((n) & 0xFF0000UL) >> 8) | (((n) & 0xFF000000UL) >> 24);
#define BL_NTOHL(n)    BL_HTONL(n)


#define PRAGMA(x) _Pragma(#x)
#define HAL_MCU_DATA_ALIGN(n) //PRAGMA(data_alignment=n)

#define BL_POLL()   LinkInternalProcess(); BLWdgReload();


/* �豸����������� */
typedef struct {
	uint16_t LinkAddr;     // �豸��ַ
	uint16_t LinkNetId;    // �豸�����ַ
	uint32_t LinkArealId;  // �豸�����
	bool enableRoute;      // �Ƿ�߱�·�ɹ���
} LinkParDef;

/* �ƿ��Ʋ������� */
typedef struct {
	uint8_t groupId;    // ��ID
	uint8_t Leval;      // ����
	uint32_t ON_seconds; // ����ʱ��/s
	uint32_t send_peroid; // ���Ϳ����źŵ���С����
} LightParDef;

// �豸�����ṹ�嶨��
typedef struct {
	uint32_t init_flag;
	LinkParDef net_config;
	LightParDef ctr_config;
	uint8_t rf_channel;      // �豸ʹ�õ�RF�ŵ�
	uint8_t boot_rf_channel;// �����������ŵ�
}imd_config_t;


/*
 *******************************************************************************
 *                                 FUNCTIONS
 *******************************************************************************
 */
/*
 *******************************************************************************
  @brief    ��ʼ��Flash�豸

  @params   ��

  @return   �� BLResult_t ����
 *******************************************************************************
 */
BLResult_t BLPortInit(void);

/*
 *******************************************************************************
  @brief   ����1�붨ʱ��

  @params  timerCallback - ��ʱ�ص�����

  @return   ��
 *******************************************************************************
 */
void BLStartTimer( void (* timerCallback)(void) );

/*
 *******************************************************************************
  @brief    ֹͣ1�붨ʱ��

  @params   ��

  @return   ��
 *******************************************************************************
 */
void BLStopTimer();

/*
 *******************************************************************************
  @brief    Flash����

  @params   addr   - ������ʼ��ַ
            length - ��������

  @return   �� BLResult_t ����
 *******************************************************************************
 */
BLResult_t BLFlashErase(uint32_t addr, uint32_t length);

/*
 *******************************************************************************
  @brief    Flashд���ݲ���

  @params   addr   - д������ʼ��ַ
            data   - ��д�������
            length - ��д������ݳ���

  @return   �� iap_result_t ����
 *******************************************************************************
 */
BLResult_t BLFlashWrite(uint32_t addr, uint8_t *data, uint32_t length);

/*
 *******************************************************************************
  @brief    Flash�����ݲ���

  @params   addr   - ��ȡ���ݵ���ʼ��ַ
            data   - ���ݴ����Buffer
            length - ��ȡ�����ݳ���

  @return   �� BLResult_t ����
 *******************************************************************************
 */
BLResult_t BLFlashRead(uint32_t addr, uint8_t *buffer, uint32_t length);

/*
 *******************************************************************************
  @brief    �ײ㷢�����ݰ�

  @params   data  - ���͵ı���
	    length- ���ĳ���

  @return   ʵ�ʷ��͵��ֽ���������ʧ�ܷ���0
 *******************************************************************************
 */
uint16_t BLLowDataSend(uint8_t *data, uint16_t length);

/*
 *******************************************************************************
  @brief    ����Ӧ�ó���

  @params   ��

  @return   ��
 *******************************************************************************
 */
void BLStartApplication(void);

/*
 *******************************************************************************
  @brief    ����MCU

  @params   ��

  @return   ��
 *******************************************************************************
 */
void BLSystemReboot(void);

/*
 *******************************************************************************
  @brief    ��λ���Ź�

  @params   ��

  @return   ��
 *******************************************************************************
 */
void BLWdgReload(void);
#endif



