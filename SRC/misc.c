#include "config.h"  
//System RCC Config
void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus;
    /* Rest RCC config */
    RCC_DeInit();
    /* Enable External high-speed crystal oscillator */
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);                   
        /* PCLK2 = HCLK = 72MHz*/ 
        RCC_PCLK2Config(RCC_HCLK_Div1);
        /* PCLK1 = HCLK/2 =72/2 = 36MHz*/
        RCC_PCLK1Config(RCC_HCLK_Div2);
        /* 8Mhz * 9 = 72Mhz */ 
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); 
        /* Enable PLL */
        RCC_PLLCmd(ENABLE); 
        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {;}
        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08)
        {;}
    }
}
void Time3Init(u16 Period,u16 Prescaler)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period=Period-1;      //Period
    TIM_TimeBaseStructure.TIM_Prescaler=Prescaler;  //Prescaler
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);        //ENABLE TIM3 Interrupt
    TIM_Cmd(TIM3,ENABLE);                           //ENABLE TIM3
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

}

void delay_us(u16 time)
{    
   u32 i;  
   while(time--)
   {
      i=8;
      while(i--){};
        __nop();
        __nop();
        __nop();
        __nop();
   }
}
void delay_ms(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=7950;  
      while(i--) ;    
   }
}


