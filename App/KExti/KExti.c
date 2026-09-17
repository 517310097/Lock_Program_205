#include "KExti.h"

__IO u8 edgeEvent = 0;  // 1表示A超前，2表示B超前，0无效

void KExtiCfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef ExtiStr;
	NVIC_InitTypeDef Nvic_Str;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource9);
	
	ExtiStr.EXTI_Line = EXTI_Line8;
	ExtiStr.EXTI_LineCmd = ENABLE;
	ExtiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	ExtiStr.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&ExtiStr);
	
	ExtiStr.EXTI_Line = EXTI_Line9;
	ExtiStr.EXTI_LineCmd = ENABLE;
	ExtiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	ExtiStr.EXTI_Trigger = EXTI_Trigger_Rising;	
	EXTI_Init(&ExtiStr);
	
	Nvic_Str.NVIC_IRQChannel = EXTI9_5_IRQn;
	Nvic_Str.NVIC_IRQChannelCmd=ENABLE;
	Nvic_Str.NVIC_IRQChannelPreemptionPriority=0;
	Nvic_Str.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&Nvic_Str);
}





