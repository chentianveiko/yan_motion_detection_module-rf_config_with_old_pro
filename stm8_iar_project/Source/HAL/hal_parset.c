#include "hal_parset.h"
#include "hal_timer.h"
#include "hal_flash.h"
//#include "hal_uart.h"
#include "../Startup/main.h"

/*
 *******************************************************************************
 *                               global   variable
 *******************************************************************************
 */

/*
 *********************************************************************************
 函数功能: 计算一个配置命令帧的异或校验值
 函数参数: parset -- 配置帧结构体
 函数返回: 异或校验值(uint8_t)
 *********************************************************************************
 */
uint8_t HalXorMake(ParSettingDef_t parset) {
	uint8_t xor_tmp, i;

	xor_tmp = 0;
	xor_tmp ^= parset.cmd;
	xor_tmp ^= (uint8_t)(parset.length >> 8);
	xor_tmp ^= (uint8_t)(parset.length);
	for (i = 0; i < parset.length; i++) {
		xor_tmp ^= parset.data[i];
	}
	xor_tmp = ~xor_tmp;

	return xor_tmp;
}

/*
 *********************************************************************************
 函数功能: 当串口接收完一个数据帧后，调用此函数可以检测异或值是否正确
 函数参数: parset -- 串口接收到的数据帧结构体
 函数返回: true -- 异或校验通过  false -- 异或校验失败
 *********************************************************************************
 */
bool HalXorCheck(ParSettingDef_t parset) {
	uint8_t xor_tmp;

	xor_tmp = HalXorMake(parset);

	if (xor_tmp == parset.xor) {
		return true;
	}

	return false;
}

/*
 *********************************************************************************
 函数功能: 计算一个配置命令帧的和校验值，使用此函数前请确认结构体的Xor值已计算
 函数参数: parset -- 配置帧结构体
 函数返回: 和校验值(uint8_t)
 *********************************************************************************
 */
uint8_t HalSumMake(ParSettingDef_t parset) {
	uint8_t sum_tmp, i;

	sum_tmp = 0;
	sum_tmp += parset.cmd;
	sum_tmp += (uint8_t)(parset.length >> 8);
	sum_tmp += (uint8_t)(parset.length);

	for (i = 0; i < parset.length; i++) {
		sum_tmp += parset.data[i];
	}

	sum_tmp += parset.xor;

	return sum_tmp;
}

/*
 *********************************************************************************
 函数功能: 当串口接收完一个数据帧后，调用此函数可以检测和校验值是否正确
 函数参数: parset -- 串口接收到的数据帧结构体
 函数返回: true -- 和校验通过  false -- 和校验失败
 *********************************************************************************
 */
bool HalSumCheck(ParSettingDef_t parset) {
	uint8_t sum_tmp;

	sum_tmp = HalSumMake(parset);

	if (sum_tmp == parset.sum) {
		return true;
	}

	return false;
}
