#include "config.h"	
#ifdef UsbEnable
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_istr.h"
#include "hw_config.h"
#include "usb_pwr.h"  
#include "string.h"	
#include "stdarg.h"		 
#include "stdio.h"	

 
_usb_usart_fifo uu_txfifo;					//USB uart send FIFO structural
u8  USART_PRINTF_Buffer[USB_USART_REC_LEN];	//usb_printf send buffer


u8 USB_USART_RX_BUF[USB_USART_REC_LEN]; 	//USB uart RX buffer 

//RX state
//bit15£¬	RX is OK
//bit14£¬	received 0x0d
//bit13~0£¬	A valid length is received
u16 USB_USART_RX_STA=0;       				//Receiving status flag

extern LINE_CODING linecoding;							//USB uart configuration information

//USB wake-up interrupt function
void USBWakeUp_IRQHandler(void) 
{
	EXTI_ClearITPendingBit(EXTI_Line18);//USB wake interrupt position 0
} 

//USB interrupt handler
void USB_LP_CAN1_RX0_IRQHandler(void) 
{
	USB_Istr();
} 

//USB Clock configuration ,USBclk=48Mhz@HCLK=72Mhz
void Set_USBClock(void)
{
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);//USBclk=PLLclk/1.5=48Mhz	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);	 //USB Clock ENABLE
} 

//usb enter low power mode
void Enter_LowPowerMode(void)
{
// 	printf("usb enter low power mode\r\n");
	bDeviceState=SUSPENDED;
} 

//leave low power mode
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo=&Device_Info;
//	printf("leave low power mode\r\n"); 
	if (pInfo->Current_Configuration!=0)bDeviceState=CONFIGURED; 
	else bDeviceState = ATTACHED; 
} 

//USB interrupt configuration
void USB_Interrupts_Config(void)
{ 
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

 
	/* Configure the EXTI line 18 connected internally to the USB IP */
	EXTI_ClearITPendingBit(EXTI_Line18);
											
	EXTI_InitStructure.EXTI_Line = EXTI_Line18; // USB resume from suspend mode
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//line 18 Trigger Rising
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 	 

	/* Enable the USB interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USB Wake-up interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;   
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_Init(&NVIC_InitStructure);   
}	

//USB Interface configuration, 1.5K Pull-up resistors
//NewState:DISABLE,
//         ENABLE,
void USB_Cable_Config (FunctionalState NewState)
{ 
	if (NewState!=DISABLE)
	{
//		printf("usb pull up enable\r\n"); 
	}
	else 
	{
//		printf("usb pull up disable\r\n"); 
	}
}

//USB connection Enable or DISABLE
//enable:0,DISABLE connection
//       1,Enable connection
void USB_Port_Set(u8 enable)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);   
	if(enable)_SetCNTR(_GetCNTR()&(~(1<<1)));//Exit power failure mode
	else
	{	  
		_SetCNTR(_GetCNTR()|(1<<1));  // power failure mode
		GPIOA->CRH&=0XFFF00FFF;
		GPIOA->CRH|=0X00033000;
        GPIO_ResetBits(GPIOA,GPIO_Pin_12);    		  
	}
}  

//Gets STM32 unique ID -> USB configuration information
void Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;
	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);
	Device_Serial0 += Device_Serial2;
	if (Device_Serial0 != 0)
	{
		IntToUnicode (Device_Serial0,&Virtual_Com_Port_StringSerial[2] , 8);
		IntToUnicode (Device_Serial1,&Virtual_Com_Port_StringSerial[18], 4);
	}
} 

//Converts a 32-bit value to unicode.
//value:32bit
//pbuf:Store address
//len:length 
void IntToUnicode (u32 value , u8 *pbuf , u8 len)
{
	u8 idx = 0;
	for( idx = 0 ; idx < len ; idx ++)
	{
		if( ((value >> 28)) < 0xA )
		{
			pbuf[ 2* idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2* idx] = (value >> 28) + 'A' - 10; 
		} 
		value = value << 4; 
		pbuf[ 2* idx + 1] = 0;
	}
}
 
//Display the USB configuration
bool USART_Config(void)
{
	uu_txfifo.readptr=0;	//Empty read pointer
	uu_txfifo.writeptr=0;	//Clear write pointer
	USB_USART_RX_STA=0;		//USB USART RX state ->0
//	printf("linecoding.format:%d\r\n",linecoding.format);
//  	printf("linecoding.paritytype:%d\r\n",linecoding.paritytype);
//	printf("linecoding.datatype:%d\r\n",linecoding.datatype);
//	printf("linecoding.bitrate:%d\r\n",linecoding.bitrate);
	return (TRUE);
}
 
void USB_To_USART_Send_Data(u8* data_buffer, u8 Nb_bytes)
{ 
	u8 i;
	u8 res;
	for(i=0;i<Nb_bytes;i++)
	{  
		res=data_buffer[i]; 
		if((USB_USART_RX_STA&0x8000)==0)	
		{
			if(USB_USART_RX_STA&0x4000)			
			{
				if(res!=0x0a)USB_USART_RX_STA=0;//ERROR
				else USB_USART_RX_STA|=0x8000;	
			}else 
			{	
				if(res==0x0d)USB_USART_RX_STA|=0x4000;
				else
				{
					USB_USART_RX_BUF[USB_USART_RX_STA&0X3FFF]=res;
					USB_USART_RX_STA++;
					if(USB_USART_RX_STA>(USB_USART_REC_LEN-1))USB_USART_RX_STA=0;//error
				}					
			}
		}   
	}  
} 

//USB USART SendData
void USB_USART_SendData(u8 data)
{
	uu_txfifo.buffer[uu_txfifo.writeptr]=data;
	uu_txfifo.writeptr++;
	if(uu_txfifo.writeptr==USB_USART_TXFIFO_SIZE)
	{
		uu_txfifo.writeptr=0;
	} 
}

//usb UART ->printf 
void usb_printf(char* fmt,...)  
{  
	u16 i,j;

	va_list ap;
    
	va_start(ap,fmt);
	vsprintf((char*)USART_PRINTF_Buffer,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART_PRINTF_Buffer);//send length data
	for(j=0;j<i;j++)//Cycle to send data
	{
		USB_USART_SendData(USART_PRINTF_Buffer[j]); 
	}
    
} 

#endif





















