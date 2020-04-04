#ifndef _config_H
#define _config_H

#include "stm32f10x.h"
#include "stm32f10x_it.h"

#define SbusLength 25				    // Sbus Length TX and RX

#define SbusRX
#define SbusTX

#define  UartDebug  

#define UartRC

void UartInit(void);
void delay_us(u16 time);
void delay_ms(u16 time);
void RCC_Configuration(void);
void Time3Init(u16 Period,u16 Prescaler);

#ifdef SbusTX
extern uint8_t SbusDataTx[SbusLength];
extern u16 ChannelTx[16];;
#define DataSendPeriod 11000 
//11000us = 11ms  ((11000)*(72))/72*10^6=11ms,
extern void MakeSbusPackage(u16 *Array);
#endif

#ifdef SbusRX
extern uint8_t SbusDataRx[SbusLength];
extern u16 ChannelRx[16];
extern void UnSbusPack(u16 *Array);
#endif


// LED config
#define LedPort                     GPIOC
#define LedPIN                      GPIO_Pin_13
#define LedState(State) State?GPIO_SetBits(LedPort,LedPIN):GPIO_ResetBits(LedPort,LedPIN);
#define LedXor(PIN) LedPort->ODR ^= PIN

#ifdef  UartRC                      
#define RC_BAUD                 100000

#ifdef  SbusRX
#define RC_RX_UART_Port         USART2
#define RC_RX_UART_IRQn 		USART2_IRQn
#define RC_RX_UART_RCC          RCC_APB1Periph_USART2
#define RC_RXIO_Port            GPIOA
#define RC_RXIO_RCC             RCC_APB2Periph_GPIOA
#define RC_RX_PIN               GPIO_Pin_3

#define RC_DMA_Rx
#define RC_DMA_Rx_RCC           RCC_AHBPeriph_DMA1
#define RC_DMA_Rx_Ch 			DMA1_Channel6
#define RC_Dma_RxFlagTC			DMA1_FLAG_TC6
#endif
#ifdef  SbusTX
#define RC_TX_UART_Port         USART2
#define RC_TX_UART_RCC          RCC_APB1Periph_USART2
#define RC_GPIO_Port            GPIOA
#define RC_TXIO_RCC             RCC_APB2Periph_GPIOA
#define RC_TX_PIN               GPIO_Pin_2

#define RC_DMA_Tx
#define RC_DMA_Tx_RCC           RCC_AHBPeriph_DMA1
#define RC_DMA_Tx_Ch 			DMA1_Channel7
#define RC_Dma_TxFlagTC			DMA1_FLAG_TC7
extern void UartTxDmaSend(void);
#endif

#endif

#ifdef  UartDebug                   
#define UsbEnable                   //Enable USB,USB -> VCP COM
#define UartDebug_VCP
#endif

#endif



