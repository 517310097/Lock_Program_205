#ifndef __KLIB__
#define __KLIB__
#include "stm32f10x.h"
#include "KHostMotor.h"
//大锁 6.5--16
//小锁 3--11
//整个大锁行程  需要转5.424375圈用1770516各脉冲
//29s需要用的脉冲数是1800000左右
#define MaxCount     2900      //最大计数时间是29s  单位是10ms
#define HMotorAccval 350      
#define HMotorDecval 350      
#define HMotorSpeed  785       //最快速度  750rpm
#define HMStepval    306       //6圈对应1958400个脉冲

typedef struct{
	vu8 SystemStatus;           //系统状态
	vu8 SysCmd;                 //命令
	vu8 SysCmdLast;
	u8 SysLockFlag;             //系统加解锁状态标志
	u8 SysUnLockFlag;
	u32 LockActCount;           //加锁过程计数器
	u32 UnLockActCount;         //解锁过程计数器
	u8  AllownLockCounter;      //保存定时器定时值
	u8  AllownUnLockCounter;
	u8  SystemErrNum;           //系统故障码	
    vu8 ChkCMDSourceStopFlag;   //
    vu8 ChkStaFlag;
}SystemLib;

//系统状态
typedef enum{
	SYSNONE = 0,                 //刚上电是未知态，需要判断 命令接口 和 到位状态检测一次 完成后进入SYSREADY
	SYSREADY,                    //刚上电状态 实时检测电机命令和到位情况，到位后上报，若发生故障进入故障态SYSErr
	SYSRUN,
    SYSCHKPOSITION,
	SYSErr                       //系统故障
}SYSSTA;



extern SystemLib SystemPara;

void InitSystemPara(void);
void NewPortCmdHandle(void);
void OldPortCmdHandle(void);
void ErrHandle(u8 ErrNum);
void ClearAllFlag(void);
void ChkNewCmdPort (void);
void ChkPositionSignal(void);
void MainHandle(void);
void ChkCMDSource (void);

#endif



