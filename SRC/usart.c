#include "config.h"	  
#include "stdio.h"	


void UartInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;    

#ifdef  UartRC	
    
#ifdef SbusTX 
    RCC_APB2PeriphClockCmd(RC_TXIO_RCC,ENABLE);
    RCC_APB1PeriphClockCmd(RC_TX_UART_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = RC_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif 
    
#ifdef SbusRX
    RCC_APB2PeriphClockCmd(RC_RXIO_RCC,ENABLE);
    RCC_APB1PeriphClockCmd(RC_RX_UART_RCC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = RC_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(RC_RXIO_Port, &GPIO_InitStructure);
#endif   

   //USART init config
	USART_InitStructure.USART_BaudRate = RC_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;//8b Not identified by betaflight 
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
#ifdef SbusRX    
	USART_InitStructure.USART_Mode = USART_Mode_Rx ;
	USART_ITConfig(RC_RX_UART_Port, USART_IT_IDLE, ENABLE);
    USART_Init(RC_RX_UART_Port, &USART_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = RC_RX_UART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	
	
	USART_DMACmd(RC_RX_UART_Port,USART_DMAReq_Rx,ENABLE);
    USART_Cmd(RC_RX_UART_Port, ENABLE);
#endif 
#ifdef SbusTX    
	USART_InitStructure.USART_Mode |= USART_Mode_Tx ;
    USART_Init(RC_TX_UART_Port, &USART_InitStructure);
    USART_Cmd(RC_TX_UART_Port, ENABLE);
#endif  

    
#ifdef RC_DMA_Rx    //Open RX DMA, RC --> UartRC_RX 

    RCC_AHBPeriphClockCmd(RC_DMA_Rx_RCC,ENABLE);                       
	  
    DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)(&RC_RX_UART_Port->DR);         
    DMA_InitStructure.DMA_MemoryBaseAddr     =  (u32) SbusDataRx;            
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                     
    DMA_InitStructure.DMA_BufferSize = SbusLength;                                
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;       
    DMA_InitStructure.DMA_MemoryInc =DMA_MemoryInc_Enable;                  
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;        
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                       
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                    
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                           
    DMA_Init(RC_DMA_Rx_Ch,&DMA_InitStructure);   
	DMA_ClearFlag(RC_Dma_RxFlagTC);
	DMA_Cmd(RC_DMA_Rx_Ch,ENABLE);                    
#endif

#ifdef SbusTX    //Open TX DMA, UartRC_TX --> FC

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_DeInit(RC_DMA_Tx_Ch);
		
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&RC_TX_UART_Port->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SbusDataTx;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = SbusLength;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;		//You cannot use DMA Mode Circular or you will have a problem,
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(RC_DMA_Tx_Ch, &DMA_InitStructure);
    USART_DMACmd(RC_TX_UART_Port,USART_DMAReq_Tx,ENABLE); 
    DMA_Cmd(RC_DMA_Tx_Ch, DISABLE);
	
#endif

#endif

}
#ifdef SbusTX  //Open TX DMA, UartRC_TX --> FC
void MakeSbusPackage(u16 *Array)
{
    SbusDataTx[0]  = 0x0f;
    SbusDataTx[1]  =  Array[0]  & 0xff;
    SbusDataTx[2]  = (Array[1]  << 3 | Array[0]  >> 8  ) & 0xff;
    SbusDataTx[3]  = (Array[2]  << 6 | Array[1]  >> 5  ) & 0xff;
    SbusDataTx[4]  = (Array[2]  >> 2 ) & 0xff;
    SbusDataTx[5]  = (Array[3]  << 1 | Array[2]  >> 10 ) & 0xff;
    SbusDataTx[6]  = (Array[4]  << 4 | Array[3]  >> 7  ) & 0xff;
    SbusDataTx[7]  = (Array[5]  << 7 | Array[4]  >> 4  ) & 0xff;
    SbusDataTx[8]  = (Array[5]  >> 1 ) & 0xff;
    SbusDataTx[9]  = (Array[6]  << 2 | Array[5]  >> 9  ) & 0xff;
    SbusDataTx[10] = (Array[7]  << 5 | Array[6]  >> 6  ) & 0xff;
    SbusDataTx[11] = (Array[7]  >> 3 ) & 0xff;
    SbusDataTx[12] = (Array[8]  >> 0 ) & 0xff;
    SbusDataTx[13] = (Array[9] << 3 | Array[8]  >> 8  ) & 0xff;
    SbusDataTx[14] = (Array[10] << 8 | Array[9] >> 5  ) & 0xff;
    SbusDataTx[15] = (Array[10] >> 2 ) & 0xff;
    SbusDataTx[16] = (Array[11] << 1 | Array[10] >> 10 ) & 0xff;
    SbusDataTx[17] = (Array[12] << 4 | Array[11] >> 7  ) & 0xff;
    SbusDataTx[18] = (Array[13] << 7 | Array[12] >> 4 ) & 0xff;
    SbusDataTx[19] = (Array[13] >> 1 ) & 0xff;
    SbusDataTx[20] = (Array[14] << 2 | Array[13] >> 9 ) & 0xff;
    SbusDataTx[21] = (Array[15] << 5 | Array[14] >> 6 ) & 0xff;
    SbusDataTx[22] = (Array[15] >> 3 ) & 0xff;
    SbusDataTx[23] = 0x00;
    SbusDataTx[24] = 0x00;
    
}
void UartTxDmaSend(void) 
{ 
	DMA_Cmd(RC_DMA_Tx_Ch, DISABLE );	//DISABLE RC_DMA_Tx_Ch
 	DMA_SetCurrDataCounter(RC_DMA_Tx_Ch,SbusLength); //send DataNumber
 	DMA_Cmd(RC_DMA_Tx_Ch, ENABLE);		
}
#endif
#ifdef SbusRX	
void UnSbusPack(u16 *Array)  //sbus Decode
{	
	Array[0]  = ((SbusDataRx[2]  & 0x07)<<8) | (SbusDataRx[1] & 0x7ff);                                    //3+8
	Array[1]  = ((SbusDataRx[3]  & 0x3f)<<5) | ((SbusDataRx[2] >>3) & 0x1F);                               //6+5
	Array[2]  = (((SbusDataRx[5]  & 0x01)<<10)| (SbusDataRx[4] <<2) | (SbusDataRx[3]>>6) ) & 0x7ff;        //1+8+2
	Array[3]  = (((SbusDataRx[6]  & 0x0f)<<7) | (SbusDataRx[5] >>1)  ) & 0x7ff;                            //4+7
	Array[4]  = (((SbusDataRx[7]  & 0x7f)<<4) | (SbusDataRx[6] >>4)  ) & 0x7ff;                            //7+4
	Array[5]  = (((SbusDataRx[9]  & 0x03)<<9) | (SbusDataRx[8] <<1)  | (SbusDataRx[7]>>7) ) & 0x7ff;       //2+8+1
	Array[6]  = (((SbusDataRx[10] & 0x1f)<<6) | (SbusDataRx[9] >>2)  ) & 0x7ff;                            //5+6
	Array[7]  = ((SbusDataRx[11]<<3) | (SbusDataRx[10]>>5)) & 0x7ff;                                       //8+3 
	Array[8]  = (((SbusDataRx[13] & 0x07)<<8) | SbusDataRx[12]) & 0x7ff;                                   //3+8
	Array[9]  = (((SbusDataRx[14] & 0x3f)<<5) | (SbusDataRx[13] >>3) ) & 0x7ff;                            //6+5
	Array[10] = (((SbusDataRx[16] & 0x01)<<10)| (SbusDataRx[15] <<2) | (SbusDataRx[14]>>6) )& 0x7ff;       //1+8+2
	Array[11] = (((SbusDataRx[17] & 0x0f)<<7) | (SbusDataRx[16] >>1) ) & 0x7ff;                            //4+7
	Array[12] = (((SbusDataRx[18] & 0x7f)<<4) | (SbusDataRx[17] >>4) ) & 0x7ff;                            //7+4
	Array[13] = (((SbusDataRx[20] & 0x03)<<9) | (SbusDataRx[19] <<1) | (SbusDataRx[18]>>7)) & 0x7ff;       //2+8+1
	Array[14] = (((SbusDataRx[21] & 0x1f)<<6) | (SbusDataRx[20] >>2) ) & 0x7ff;                            //5+6      
	Array[15] = ((SbusDataRx[22]<<3) | (SbusDataRx[21]>>5)) & 0x7ff;    								   //8+3
	SbusDataRx[0]=0;
	SbusDataRx[24]=0xff;
}
int DMAbuff_length=0; 
void USART2_IRQHandler(void) 
{
	u8 RxTemp;
	if(USART_GetITStatus(RC_RX_UART_Port,USART_IT_IDLE)!=RESET)
	{
		RxTemp=RC_RX_UART_Port->SR;			// USART2->SR
		RxTemp=RC_RX_UART_Port->DR;			// Clear USART2->DR
		DMA_Cmd(RC_DMA_Rx_Ch,DISABLE);		// off DMA1_Channel6
		DMA_ClearFlag(RC_Dma_RxFlagTC);		// Clear DMA1_FLAG_TC6
		DMAbuff_length=DMA_GetCurrDataCounter(RC_DMA_Rx_Ch);//Get the receive length
		if(DMAbuff_length!=SbusLength || (SbusDataRx[0] != 0x0f && SbusDataRx[24]!=0x00))// Invalid frame,Set Curr Data Counter SbusLength
		{
			DMA_SetCurrDataCounter(RC_DMA_Rx_Ch,SbusLength);
			SbusDataRx[0]=0;
			SbusDataRx[24]=0;
		}
		else
		{
			UnSbusPack(ChannelRx);//	Valid frame,The received sbus data is parsed and stored in ChannelRx
		}
		DMA_Cmd(RC_DMA_Rx_Ch,ENABLE);
	}
}
#endif	 


