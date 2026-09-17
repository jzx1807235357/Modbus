/**
  ******************************************************************************
  * @file    modbus_master_test.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2020-xx-xx
  * @brief   modbus测试
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "modbus_master_test.h"
#include "mb_m.h"
#include "mbport.h"
#include "user_mb_app.h"
/**
  * @brief  测试程序
  * @param  功能选择
  * @retval 无
  */
void test(char MB)
{
	USHORT Hlod_buff[4];
	UCHAR	Coils[4]={1,0,1,0};
	
	Hlod_buff[0] = HAL_GetTick() & 0xff;		           //获取时间戳 提出1至8位
	Hlod_buff[1] = (HAL_GetTick() & 0xff00) >> 8;      //获取时间戳 提出9至16位
	Hlod_buff[2] = (HAL_GetTick() & 0xff0000) >> 16 ;  //获取时间戳 提出17至24位
	Hlod_buff[3] = (HAL_GetTick() & 0xff000000) >> 24; //获取时间戳 提出25至32位
	
	/* 注：各操作的API在mb_m.h中 */
	switch(MB)
	{
		case MB_USER_HOLD: 
					/* 写多个保持寄存器值 */
					eMBMasterReqWriteMultipleHoldingRegister(MB_SAMPLE_TEST_SLAVE_ADDR, //从机设备地址
																									 MB_REG_START, 							//数据起始位置
																									 MB_SEND_REG_NUM, 					//写数据总数
																									 Hlod_buff, 								//数据
																									 WAITING_FOREVER);					//永久等待
		break;
		
		case MB_USER_COILS:
					/* 写多个线圈 */
					eMBMasterReqWriteMultipleCoils(MB_SAMPLE_TEST_SLAVE_ADDR, //从机设备地址
																				 MB_REG_START, 							//数据起始位置
																				 MB_SEND_REG_NUM, 					//写数据总数
																				 Coils, 										//数据
																				 WAITING_FOREVER);					//永久等待
		break;
		
		case MB_USER_INPUT_REG:
					/* 读输入寄存器 */
					eMBMasterReqReadInputRegister(MB_SAMPLE_TEST_SLAVE_ADDR,	//从机设备地址
																				MB_REG_START,               //数据起始位置
																				MB_READ_REG_NUM,						//读数据总数
																				WAITING_FOREVER);						//永久等待
		break;
	}
}	



