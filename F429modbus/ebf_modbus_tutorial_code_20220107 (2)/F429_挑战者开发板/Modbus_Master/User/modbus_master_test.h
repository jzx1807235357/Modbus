#ifndef __MODBUS_MASTER_TEST_H
#define __MODBUS_MASTER_TEST_H

/* 测试功能参数 */
#define MB_SAMPLE_TEST_SLAVE_ADDR						1			//从机设备地址
#define MB_REG_START  											1			//发送数据起始位置
#define MB_SEND_REG_NUM    									4			//发送数据数量
#define MB_READ_REG_NUM    									4			//接收数据数量
#define WAITING_FOREVER											-1		//永久等待

/* 通过宏定义选择功能 */
#define MB_USER_HOLD 												1			//写多个保持寄存器
#define MB_USER_COILS 											2			//写多个线圈
#define MB_USER_INPUT_REG										3			//读输入寄存器

extern void test(char MB);												//测试程序

#endif
