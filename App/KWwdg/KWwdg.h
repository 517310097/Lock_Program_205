#ifndef __kwwdg_H
#define __kwwdg_H
#include "stm32f10x.h"

void WWDG_Init(u8 tr,u8 wr,u32 fprer);//初始化WWDG
void WWDG_Set_Counter(u8 cnt);          //设置WWDG的计数器
void WWDG_NVIC_Init(void);

#endif
