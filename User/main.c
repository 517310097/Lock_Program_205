#include "kpublic.h"  

#ifdef UartDebug
uint8_t LockCtrCnt = 0;
uint8_t UnlockCtrCnt = 0;
uint8_t IdleCtrCnt = 0;
#endif


void Report_status(void)
{
    if(SystemPara.SystemStatus != SYSErr)             
    {
        if((SystemPara.SysLockFlag == PositionEn)&&(SystemPara.LockActCount < MaxCount))		  /*加锁到位了，解锁未到位，枷锁运行时间未超出，加锁完成*/
        {	
            #ifdef UartDebug
            if(!LockCtrCnt)
            {
                printf("LockOk&Output\r\n");
                LockCtrCnt++;
                IdleCtrCnt = 0;
            }
            #endif	
            OUTPUT_POSITION_STATUS_Lock;                  
        }

        if((SystemPara.SysUnLockFlag == PositionEn)&&(SystemPara.UnLockActCount < MaxCount))/*解锁到位了，加锁未到位，解锁完成*/
        {
            #ifdef UartDebug
            if(!UnlockCtrCnt)	
            {						
                printf("UnlockOk&Output\r\n");
                UnlockCtrCnt++;
                IdleCtrCnt = 0;
            }
            #endif						
            OUTPUT_POSITION_STATUS_Unlock;
        }		
        
        if((SystemPara.SysUnLockFlag == PositionDis)&&(SystemPara.SysLockFlag == PositionDis))/*解锁到位了，加锁未到位，解锁完成*/
        {
            #ifdef UartDebug
            if(!IdleCtrCnt)	
            {						
                printf("Idle_Mod\r\n");
                IdleCtrCnt++;
                LockCtrCnt = 0;
                UnlockCtrCnt = 0;						
            }
            #endif						
            Output_Position_Status_Idle;
        }					
    }
//    else                                                  //20260531   205电话会议对接  故障状态不输出
//    {
//        OUTPUT_POSITION_STATUS_Err;                      //在外层 告诉主控发生了故障
//    }       
}

int main()
{
	KsystickCfg();
    KUartCfg();	
    MspIOCfg();
    KExtiCfg();
	StepMotorCfg();
	
	InitSystemPara();
  
	WWDG_Init(0x7f,0X5f,WWDG_Prescaler_8);
	#ifdef UartDebug
	printf("H H :Idle_Mod\r\n");
	#endif		
	while(1)
	{
						
		/*检测到驱动芯片故障引脚上报故障*/
		if(MspIOStr.FaultPinFlag)
		{
			SystemPara.SystemStatus = SYSErr; 		            //立马进入故障状态
			SystemPara.SystemErrNum = 3;                        //故障码
		}
		MainHandle();
		
		if(task1ms)
		{
		    task1ms = 0;
			if(SystemPara.SystemStatus == SYSREADY)
			{
			    if(MspIOStr.UseCmdPortFlag == CmdOldPort)
					OldPortCmdHandle();	
			}				
		}
		
		if(task10ms)
		{
			task10ms = 0;                                       //20260910去掉10任务ChkPositionSignal
            ChkCMDSource();                                     //20260910
            if(SystemPara.SystemStatus == SYSNONE)
                ChkPositionSignal();
			ChkFaultSignal();                                   //检测驱动芯片故障脚
			if(SystemPara.SystemStatus == SYSREADY)
			{
			    if(MspIOStr.UseCmdPortFlag == CmdNewPort)
					NewPortCmdHandle();
			}
            Report_status();
			if(SystemPara.AllownLockCounter == TRUE)
				SystemPara.LockActCount++;	
			if(SystemPara.AllownUnLockCounter == TRUE)
				SystemPara.UnLockActCount++;
		}
        
        if(task500ms)                                           //20260910添加500任务ChkPositionSignal
        {
            task500ms = 0;
            if(SystemPara.SystemStatus != SYSNONE)
                ChkPositionSignal();
        }
	}
}

