#ifndef __KEXTI__
#define __KEXTI__

#include  "stm32f10x.h"

#define ReadPinB()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9) 
#define ReadPinA()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)


extern __IO u8 edgeEvent;  // 1表示A超前，2表示B超前，0无效

void KExtiCfg(void);

#endif


