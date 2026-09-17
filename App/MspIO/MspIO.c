#include "MspIO.h"
#include "kuart.h"
MspIOStruct MspIOStr={0};

void MspIOCfg(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;  
	 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = ExtendPin|RetractPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ExtendCmdPin|RetractCmdPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = FaultPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	FaultPinInit;
	
	GPIO_InitStructure.GPIO_Pin = ChkCmdPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ExtendSignalOutPin|RetractSignalOutPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  Output_Position_Status_Idle;	
}



void ChkFaultSignal(void)
{
	//检测上锁到位信号
	if(ReadFaultPin == Bit_RESET)
	{	
		MspIOStr.ChkFaultPinCnt++;
		if(MspIOStr.ChkFaultPinCnt>=2)
		{
		    MspIOStr.FaultPinFlag = 1;
			MspIOStr.ChkFaultPinCnt = 0;
		}
	}
	else
		MspIOStr.ChkFaultPinCnt = 0;
}



