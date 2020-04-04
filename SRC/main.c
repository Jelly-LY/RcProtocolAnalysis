//#include "SPI.h"  
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"	 
#include "config.h"
#include "stdio.h"



#ifdef SbusRX
u16 ChannelRx[16];
uint8_t SbusDataRx[SbusLength]={0};
#endif

#ifdef SbusTX
u16 ChannelTx[16];
uint8_t SbusDataTx[SbusLength]={0};
#endif

#ifdef LedPort
void LedInit()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = LedPIN;              
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LedPort, &GPIO_InitStructure);
	GPIO_ResetBits(LedPort,LedPIN);
}
void LedFlash(u8 Num)   //LED flash 
{
    u8 i,j;
    for(i = 1;i<=Num;i++)
    {
        for(j = i;j>0;j--)
        {
            LedState(0);
            delay_ms(150); 
            LedState(1);
            delay_ms(150);
        }
        delay_ms(200);
    }
}
#endif

void System_Init()
{
    RCC_Configuration();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);        // GPIOA clock 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);        // GPIOB clock 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);        // GPIOC clock 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);        // AFIO  clock 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE );   // JTAG Disable

#ifdef LedPort    
    LedInit();	        // Led Init 
//    LedFlash(3);        // Led Flash 1  11  111 
#endif
    UartInit();         // UART Init
#ifdef  UartDebug
#ifdef UsbEnable
    USB_Port_Set(0); 	        //USB disconnect
    delay_ms(700);      
    USB_Port_Set(1);	        //USB connect
    Set_USBClock();             //USB clock init
    USB_Interrupts_Config();    //USB Interrupts init
    USB_Init();	                //USB init
    delay_ms(1000);
    usb_printf("USB VCP init is OK !");
#endif
#endif 
}


int main(void)
{	
    System_Init();
#ifdef  SbusTX
    Time3Init(DataSendPeriod,71);
#endif 	
	while(1)
    {
#ifdef  SbusRX
#ifdef  UartDebug
        for(u8 i=0;i<15;i++)
        usb_printf("CH[%d]",(int)i,ChannelRx[i]);
#endif 
#endif 
    }
	
}
#if defined (SbusTX)// Sbus send
void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
    {
        TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
        // sbus update out
		LedXor(LedPIN);
#if defined(SbusTX) && defined(SbusRX)//Define both SbusRX and SbusTX and copy the decoded data
		for(int i1=0;i1<16;i1++)
		{
			ChannelTx[i1]=ChannelRx[i1];
		}
#endif
		MakeSbusPackage(ChannelTx);// Make the sbus package with the ChannelTx data.
        UartTxDmaSend();  // Uart Rx Dma Send: RX->FC
    }
}
#endif




