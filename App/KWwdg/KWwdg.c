#include "Kwwdg.h"

u8 WWDG_CNT=0x7f; 

void WWDG_Init(u8 tr,u8 wr,u32 fprer)
{ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);  //WWDG时钟使能

	WWDG_CNT = tr&WWDG_CNT;
  
	WWDG_SetPrescaler(fprer);															//设置IWDG预分频值

	WWDG_SetWindowValue(wr);															//设置窗口值

	WWDG_Enable(WWDG_CNT);	 															//使能看门狗 ,	设置 counter .                  

	WWDG_ClearFlag();																			//清除提前唤醒中断标志位 

	WWDG_NVIC_Init();																			//初始化窗口看门狗 NVIC

	WWDG_EnableIT(); 																			//开启窗口看门狗中断
} 
//重设置WWDG计数器的值
void WWDG_Set_Counter(u8 cnt)
{
    WWDG_Enable(cnt);	 
}
//窗口看门狗中断服务程序
void WWDG_NVIC_Init()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;    //WWDG中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   //抢占2，子优先级3，组2	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 //抢占2，子优先级3，组2	
  NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; 
	NVIC_Init(&NVIC_InitStructure);//NVIC初始化
}

//窗口看门狗中断函数
void WWDG_IRQHandler(void)
{
  WWDG_SetCounter(WWDG_CNT);	  //当禁掉此句后,窗口看门狗将产生复位
	WWDG_ClearFlag();	            //清除提前唤醒中断标志位
}

