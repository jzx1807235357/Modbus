/*
 * FreeModbus Libary: RT-Thread Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "usart.h"
#include "stm32f4xx.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Defines ------------------------------------------*/
/* serial transmit event */
#define EVENT_SERIAL_TRANS_START    (1<<0)

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);

//static void serial_soft_trans_irq(void* parameter);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
        eMBParity eParity)
{
    /**
     * set 485 mode receive and transmit control IO
     * @note MODBUS_MASTER_RT_CONTROL_PIN_INDEX need be defined by user
     */
	
		/* 使用485时需要在usart.h中打开RT_MODBUS_MASTER_USE_CONTROL_PIN宏定义 */
	#if defined(MODBUS_MASTER_USE_CONTROL_PIN)	
		modbus_master_control_init();
	#endif
	
	/* 串口2初始化 */
  MX_USART2_UART_Init(ucPORT,ulBaudRate,eParity);
	
	return TRUE;
}

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	  /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
		if(xRxEnable)
			{
				/* 串口2接收中断使能 */
				__HAL_USART_ENABLE_IT(&huart2,USART_IT_RXNE); 
				#if defined(MODBUS_MASTER_USE_CONTROL_PIN)	
					/* 485低电平接收 */
					HAL_GPIO_WritePin(MODBUS_MASTER_GPIO_PORT,MODBUS_MASTER_GPIO_PIN,MODBUS_MASTER_GPIO_PIN_LOW);
				#endif
			}
			else
			{
				/* 串口2接收中断关闭 */
				__HAL_USART_DISABLE_IT(&huart2,USART_IT_RXNE); 
				#if defined(MODBUS_MASTER_USE_CONTROL_PIN)
					/* 485高电平发送 */
					HAL_GPIO_WritePin(MODBUS_MASTER_GPIO_PORT,MODBUS_MASTER_GPIO_PIN,MODBUS_MASTER_GPIO_PIN_HIGH);
				#endif
			}
			
			if(xTxEnable)
			{
				/* 串口2发送中断使能 */
				__HAL_USART_ENABLE_IT(&huart2,USART_IT_TXE); 
				#if defined(MODBUS_MASTER_USE_CONTROL_PIN)
					/* 485高电平发送*/
					HAL_GPIO_WritePin(MODBUS_MASTER_GPIO_PORT,MODBUS_MASTER_GPIO_PIN,MODBUS_MASTER_GPIO_PIN_HIGH);
				#endif
			}
			else
			{
				/* 串口2发送中断关闭 */
				__HAL_USART_DISABLE_IT(&huart2,USART_IT_TXE); 
				#if defined(MODBUS_MASTER_USE_CONTROL_PIN)	
					/* 485低电平接收*/
					HAL_GPIO_WritePin(MODBUS_MASTER_GPIO_PORT,MODBUS_MASTER_GPIO_PIN,MODBUS_MASTER_GPIO_PIN_LOW);
				#endif
			}
}

void vMBMasterPortClose(void)
{
	/* 关闭串口 */
	HAL_UART_MspDeInit(&huart2);
    return;
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
	/* 发送一个字节 */
	if(HAL_UART_Transmit (&huart2 ,(uint8_t *)&ucByte,1,0x01) != HAL_OK )	
		return FALSE ;
	else
		return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR * pucByte)
{
	/* 接收一个字节 */
	if(HAL_UART_Receive (&huart2 ,(uint8_t *)pucByte,1,0x01) != HAL_OK )//添加接收一位代码
			return FALSE ;
	else
		return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
	/* 发送状态机 */
    pxMBMasterFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
		/* 接收状态机 */
    pxMBMasterFrameCBByteReceived();
}

/**
 * Software simulation serial transmit IRQ handler.
 *
 * @param parameter parameter
 */
//static void serial_soft_trans_irq(void* parameter) {
//	extern char Transmit_Flag ;
//    while (1)
//    {
//			
//        /* waiting for serial transmit start */
//				if( Transmit_Flag == 1)
//        /* execute modbus callback */
//        prvvUARTTxReadyISR();
//    }
//}

/**
 * This function is serial receive callback function
 *
 * @param dev the device of serial
 * @param size the data size that receive
 *
 * @return return RT_EOK
 */
//static char serial_rx_ind(void* dev, void* size) {
//    prvvUARTRxISR();
//    return 1;
//}
/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
	if(__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE)!= RESET) 
		{
			/* 接收中断 */
			prvvUARTRxISR();
		}

	if(__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_TXE)!= RESET) 
		{
			/* 发送中断 */
			prvvUARTTxReadyISR();
		}
	
  HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE END USART2_IRQn 1 */
}
#endif
