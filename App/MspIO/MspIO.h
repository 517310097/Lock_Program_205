#ifndef __MSPIO__
#define __MSPIO__
#include  "stm32f10x.h"

//GPIOA
#define ExtendPin     	  GPIO_Pin_1
#define RetractPin    	  GPIO_Pin_2

#define ExtendCmdPin      GPIO_Pin_3
#define RetractCmdPin 		GPIO_Pin_4

#define ExtendSignalOutPin      GPIO_Pin_6
#define RetractSignalOutPin 		GPIO_Pin_5

#define ChkCmdPin 		    GPIO_Pin_7
//B
#define FaultPin 		      GPIO_Pin_5

#define ReadChkCmdPin     GPIO_ReadInputDataBit(GPIOA,ChkCmdPin)
#define ReadExtendPin     GPIO_ReadInputDataBit(GPIOA,ExtendPin)
#define ReadRetractPin    GPIO_ReadInputDataBit(GPIOA,RetractPin)
#define ReadExtendCmdPin  GPIO_ReadInputDataBit(GPIOA,ExtendCmdPin)
#define ReadRetractCmdPin GPIO_ReadInputDataBit(GPIOA,RetractCmdPin)
#define ReadFaultPin      GPIO_ReadInputDataBit(GPIOB,FaultPin)

#define FaultPinInit      GPIO_SetBits(GPIOB,FaultPin)

#define Output_Position_Status_Idle {GPIO_SetBits(GPIOA,ExtendSignalOutPin);\
	                                   GPIO_SetBits(GPIOA,RetractSignalOutPin);}

#define OUTPUT_POSITION_STATUS_Lock {GPIO_ResetBits(GPIOA,ExtendSignalOutPin);\
	                                   GPIO_SetBits(GPIOA,RetractSignalOutPin);}
																		 
#define OUTPUT_POSITION_STATUS_Unlock {GPIO_SetBits(GPIOA,ExtendSignalOutPin);\
	                                   GPIO_ResetBits(GPIOA,RetractSignalOutPin);}
																		 
#define OUTPUT_POSITION_STATUS_Err {GPIO_ResetBits(GPIOA,ExtendSignalOutPin);\
	                                   GPIO_ResetBits(GPIOA,RetractSignalOutPin);}

																			 
typedef enum{
    CmdPortNone = 0,
	CmdNewPort,
	CmdOldPort,
}CmdPortStatus;
																		 
typedef enum{
    CmdStop = 0,
	CmdLock,
	CmdUnlock
}LockStatus;

typedef enum{
  PositionNone = 0,       //未知
	PositionEn = 1,         //到位
	PositionDis = 2,        //未到位
}PositionStatus;      

typedef struct{
	//使用新命令接口还是老命令接口
    u8 ChkOldCmdPortCnt;
	u8 UseCmdPortFlag;
	u8 ChkNewCmdPortCnt;
	
	//新命令接口命令识别
	u8 NewCmdLockCnt;
	u8 NewCmdUnlockCnt;


  //锁定到位检测
	u8 LockPositionCnt;
  //锁定到位释放检测
	u8 LockPositionReleaseCnt;	
	u8 LockPositionFlag;

  //解锁到位检测
	u8 UnLockPositionCnt;
  //解锁到位释放检测
	u8 UnLockPositionReleaseCnt;	
	u8 UnLockPositionFlag;
	
	u8 ChkFaultPinCnt;
	u8 FaultPinFlag;
}MspIOStruct;

extern MspIOStruct MspIOStr;


void MspIOCfg(void);


void ChkFaultSignal(void);




#endif


