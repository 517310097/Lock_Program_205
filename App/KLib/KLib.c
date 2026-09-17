#include "klib.h"
#include "MspIO.h"
#include "KHostMotor.h"
#include "KUart.h"
#include "KExti.h"
SystemLib SystemPara;

/*伸出和收回单行程时间设置19.6s  ，加减速各占3s，匀速3s*/
void InitSystemPara(void)
{
	SystemPara.SystemStatus = SYSNONE; 
    SystemPara.SysCmd = CmdStop;            //初始化系统命令        
	SystemPara.SysCmdLast = CmdStop;        //初始化上一次保存的系统命令
	SystemPara.SysLockFlag = PositionNone; 
	SystemPara.SysUnLockFlag = PositionNone;
	
	SystemPara.LockActCount = 0;            //保存加锁计时时间    
	SystemPara.AllownLockCounter = FALSE;   //允许加锁计时器计时
	SystemPara.UnLockActCount = 0;          //保存解锁计时时间    
	SystemPara.AllownUnLockCounter = FALSE; //允许解锁计时器计时	
	SystemPara.SystemErrNum = 0;
    SystemPara.ChkCMDSourceStopFlag = 0;
    SystemPara.ChkStaFlag =0;
}

//解析新端口发出来的命令
void NewPortCmdHandle(void)
{
	ChkNewCmdPort();
}

// 主循环中
u8 confirmedDir = 0;
u8 sameDirCount = 0;

//解析旧端口发出来的命令
void OldPortCmdHandle(void)
{
	if(edgeEvent != 0) 
	{
		if (edgeEvent == confirmedDir)       
		{
			sameDirCount++;
		} 
		else                               
		{
			sameDirCount = 0;
			confirmedDir = edgeEvent;            
		}
		edgeEvent = 0;                              // 清空事件，等待下次中断
	}
	
	if (sameDirCount >= 3) 
	{
		                                            // 执行电机控制
		if (confirmedDir == 1)
        {
			SystemPara.SysCmd = CmdLock;            //得到旧接口端传来的加锁命令
//                #ifdef UartDebug
//                    printf("PC_Tx_Cmd is CmdLock:\r\n");
//			    #endif            
        }
		else if (confirmedDir == 2) 
        {
			SystemPara.SysCmd = CmdUnlock;          //得到旧接口端传来的解锁命令
//                #ifdef UartDebug
//                    printf("PC_Tx_Cmd is CmdUnlock:\r\n");
//			    #endif             
        }
		sameDirCount = 0;                           // 避免重复执行
	}
}

void ErrHandle(u8 ErrNum)
{
	ClearAllFlag();	
	#ifdef UartDebug
		switch(ErrNum)
		{
			case 1:
			{
				static uint8_t SwErrCnt1 = 0;
				if(!SwErrCnt1)				
					printf("Sensor_err\r\n");
			}break;
			case 2:
			{
				static uint8_t SwErrCnt2 = 0;
				if(!SwErrCnt2)					
				  printf("Tim_Over_err\r\n");
			}break;
			case 3:
			{
				static uint8_t SwErrCnt3 = 0;
				if(!SwErrCnt3)					
				  printf("IC_err\r\n");
			}break;
			default:;break;
		}
	#endif		
}

void ClearAllFlag(void)
{
	HostMotorOffLine();	  		                   //电机脱机
	HostMotorStop();                             //停定时器

    srd.run_state = STOP;	                       //电机状态机进入stop                        
	srd.step_count = 0;                              //清零当前走了多少步	
	srd.step_delay = 0;  
	srd.min_delay = 0; 	
	srd.accel_count = 0;
	srd.last_accel_delay = 0;
	srd.tempi = 0;                                  
	srd.rest = 0;	
	
	SystemPara.AllownLockCounter = FALSE;        //关闭用于推杆超时限位的定时器
	SystemPara.LockActCount = 0;                 //清零用于推杆超时限位的定时器的值		
	SystemPara.AllownUnLockCounter = FALSE;      //关闭用于推杆超时限位的定时器
	SystemPara.UnLockActCount = 0;               //清零用于推杆超时限位的定时器的值	
	
}

void ChkNewCmdPort (void)
{
	//检测新命令接口的上锁命令
	if((ReadExtendCmdPin == Bit_RESET)&&(ReadRetractCmdPin == Bit_SET))
	{
        MspIOStr.NewCmdLockCnt++;
		if(MspIOStr.NewCmdLockCnt >= 3)
		{		
            SystemPara.SysCmd = CmdLock;          //得到新命令端口传来的加锁命令
			MspIOStr.NewCmdLockCnt = 0;
		}
	}
	else
        MspIOStr.NewCmdLockCnt = 0;

	//检测新命令接口的解锁命令
	if((ReadExtendCmdPin == Bit_SET)&&(ReadRetractCmdPin == Bit_RESET))	
	{
		MspIOStr.NewCmdUnlockCnt ++;
		if(MspIOStr.NewCmdUnlockCnt >= 3)
		{		
            SystemPara.SysCmd = CmdUnlock;          //得到新命令端口传来的解锁命令
			MspIOStr.NewCmdUnlockCnt = 0;
		}    		
	}
	else
		MspIOStr.NewCmdUnlockCnt = 0;
  //没有停止信号  只有到位后停。
}

void ChkPositionSignal(void)
{
	//检测上锁到位信号
	if(ReadExtendPin == Bit_SET)
	{	
		MspIOStr.LockPositionReleaseCnt = 0;
        MspIOStr.LockPositionCnt++;
		if(MspIOStr.LockPositionCnt>=5)
		{
		    MspIOStr.LockPositionFlag = PositionEn;
			MspIOStr.LockPositionCnt = 0;
		}
	}
	else
	{
		MspIOStr.LockPositionCnt = 0;
        MspIOStr.LockPositionReleaseCnt++;
		if(MspIOStr.LockPositionReleaseCnt>=2)
		{
		    MspIOStr.LockPositionFlag = PositionDis;
			MspIOStr.LockPositionReleaseCnt = 0;
		}		
	}

	//检测解锁到位信号
	if(ReadRetractPin == Bit_SET)
	{	
		MspIOStr.UnLockPositionReleaseCnt = 0;
        MspIOStr.UnLockPositionCnt++;
		if(MspIOStr.UnLockPositionCnt>=5)
		{
		    MspIOStr.UnLockPositionFlag = PositionEn;
			MspIOStr.UnLockPositionCnt = 0;
		}
	}
	else
	{
		MspIOStr.UnLockPositionCnt = 0;
        MspIOStr.UnLockPositionReleaseCnt++;
		if(MspIOStr.UnLockPositionReleaseCnt>=2)
		{
		    MspIOStr.UnLockPositionFlag = PositionDis;
			MspIOStr.UnLockPositionReleaseCnt = 0;
		}		
	}
	
    if((MspIOStr.LockPositionFlag == PositionEn)&&(MspIOStr.UnLockPositionFlag == PositionDis))
    {
        SystemPara.SysLockFlag = PositionEn;
        SystemPara.ChkStaFlag =1;
    }		
	
    if((MspIOStr.LockPositionFlag == PositionDis)&&(MspIOStr.UnLockPositionFlag == PositionEn))
    {
        SystemPara.SysUnLockFlag = PositionEn;
        SystemPara.ChkStaFlag =1;
    }	

    if((MspIOStr.LockPositionFlag == PositionEn)&&(MspIOStr.UnLockPositionFlag == PositionEn))
    {
        SystemPara.SystemStatus = SYSErr; 		                                        //进入故障状态
        SystemPara.SystemErrNum = 1;                                                    //故障码
        SystemPara.ChkStaFlag =1;
    }	

    if((MspIOStr.LockPositionFlag == PositionDis)&&(MspIOStr.UnLockPositionFlag == PositionDis))
    {
        SystemPara.SysUnLockFlag = PositionDis; 
        SystemPara.SysLockFlag = PositionDis;
        SystemPara.ChkStaFlag =1;		
    }	
}

void MainHandle(void)
{
	
    switch(SystemPara.SystemStatus)
	{
		case  SYSNONE:
		{				
			if(SystemPara.ChkCMDSourceStopFlag && SystemPara.ChkStaFlag)                //确认板子使用新命令接口还是老命令接口同时等到位信号检查完成						                                        			
				SystemPara.SystemStatus = SYSREADY; 		                            //如果上面检测完成，进入SYSREADY		
		}break;
		case SYSREADY:
		{			
			if((SystemPara.SysCmdLast != SystemPara.SysCmd)&&(SystemPara.SysCmd != CmdStop))
			{
				SystemPara.SystemStatus = SYSRUN;
				SystemPara.SysCmdLast = SystemPara.SysCmd;                              //命令被改变，更新上一次的命令状态
                #ifdef UartDebug
                if(SystemPara.SysCmd == 1)
                    printf("RX_New_Cmd is CmdLock:\r\n");
                else if(SystemPara.SysCmd == 2)
                    printf("RX_New_Cmd is CmdUnlock:\r\n");
                else if(SystemPara.SysCmd == 0)
                    printf("RX_New_Cmd is CmdStop:\r\n");
			    #endif
			}
		}break;
		case SYSRUN:
		{
			if((SystemPara.SysCmd == CmdLock)&&(SystemPara.SysLockFlag != PositionEn))
			{
				#ifdef UartDebug				
			    printf("Rx_LockCmd_And_Start\r\n");
			    #endif
				//设置圈数及加减速的参数  圈数注意正负
				STEPMOTOR_AxisMoveRel(SPR*(-HMStepval),HMotorAccval,HMotorDecval,HMotorSpeed);	//启动计算		
				SystemPara.AllownLockCounter = TRUE;         //启动定时器
                SystemPara.LockActCount = 0;                 //清零用于推杆超时限位的定时器的值				
			}
			else if((SystemPara.SysCmd == CmdUnlock)&&(SystemPara.SysUnLockFlag != PositionEn))			
			{
				#ifdef UartDebug			
			    printf("Rx_UnlockCmd_And_Start\r\n");
			    #endif				
				//设置圈数及加减速的参数  圈数注意正负
				STEPMOTOR_AxisMoveRel(SPR*HMStepval,HMotorAccval,HMotorDecval,HMotorSpeed);	//启动计算	
			    SystemPara.AllownUnLockCounter = TRUE;              //启动定时器
				SystemPara.UnLockActCount = 0;                      //清零用于推杆超时限位的定时器的值					
			} 
            HostMotorOnLine();							
			SystemPara.SystemStatus =	SYSCHKPOSITION;						
		}break;
		case SYSCHKPOSITION:
		{
			if(SystemPara.SysCmd == CmdLock)
			{
				/*25s还未上锁到位*/
				if((SystemPara.SysLockFlag == PositionDis)&&(SystemPara.LockActCount >= MaxCount))  //未上锁到位
				{					
					SystemPara.SystemStatus = SYSErr; 		         //进入故障状态	
					SystemPara.SystemErrNum = 2;	
				}
				else if((SystemPara.SysLockFlag == PositionEn)&&(SystemPara.LockActCount < MaxCount))		  /*加锁到位了，解锁未到位，枷锁运行时间未超出，加锁完成*/
				{	
	                ClearAllFlag();
					SystemPara.SysCmd = CmdStop;                     
                    SystemPara.SysCmdLast = SystemPara.SysCmd;       //命令被改变，更新上一次的命令状态
					SystemPara.SystemStatus = SYSREADY;
				}
			}
			if(SystemPara.SysCmd == CmdUnlock)
			{
				if((SystemPara.SysUnLockFlag == PositionDis)&&(SystemPara.UnLockActCount >= MaxCount))  //未上锁到位
				{
					SystemPara.SystemStatus = SYSErr; 		         //进入故障状态		
					SystemPara.SystemErrNum = 2;			
				}	
				else if((SystemPara.SysUnLockFlag == PositionEn)&&(SystemPara.UnLockActCount < MaxCount))//解锁到位
				{
                    ClearAllFlag();                                  
					SystemPara.SysCmd = CmdStop;
                    SystemPara.SysCmdLast = SystemPara.SysCmd;       //电机命令变停后没有更新SysCmdLast导致再次给解锁命令的时候不能识别，（上一步存的解锁，这里不更新的话再发解锁不认）
					SystemPara.SystemStatus = SYSREADY;	
				}
			}			
		}break;		
		case SYSErr:
		{
			ErrHandle(SystemPara.SystemErrNum);
		}break;
		default : ;break;
	}
}

//
void ChkCMDSource (void)
{
	//检测是否使用旧命令接口
	if(ReadChkCmdPin == Bit_SET)
	{
		MspIOStr.ChkNewCmdPortCnt = 0;
		MspIOStr.ChkOldCmdPortCnt ++;
		if((MspIOStr.ChkOldCmdPortCnt >= 3)&&(MspIOStr.UseCmdPortFlag == CmdPortNone))
		{		
		  MspIOStr.UseCmdPortFlag = CmdOldPort;    //使用旧命令接口
			MspIOStr.ChkOldCmdPortCnt = 0;
			#ifdef UartDebug
			  printf("Use_Old_Cmd_Port\r\n");
			#endif
			SystemPara.ChkCMDSourceStopFlag = 1;
		}
	}
	else   
	{
		MspIOStr.ChkOldCmdPortCnt = 0;	
		MspIOStr.ChkNewCmdPortCnt ++;
		if((MspIOStr.ChkNewCmdPortCnt >= 3)&&(MspIOStr.UseCmdPortFlag == CmdPortNone))
		{		
		  MspIOStr.UseCmdPortFlag = CmdNewPort;   //使用新命令接口
			MspIOStr.ChkNewCmdPortCnt = 0;
			#ifdef UartDebug
			  printf("Use_New_Cmd_Port\r\n");
			#endif			
			SystemPara.ChkCMDSourceStopFlag = 1;
		}    		
	}
}



