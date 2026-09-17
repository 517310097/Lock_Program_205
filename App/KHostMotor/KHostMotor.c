#include "KHostMotor.h"
#include "stdlib.h"	
#include "math.h"	
#include "KUart.h"
/*
共阳接法：
pul：有脉冲时工作，高电平有效（上升沿）
en：低电平脱机

1° = Π/180° = 0.01745rad
1rad = 180°/Π = 57.296°

1rpm = 0.1047rad/s   电机最快  900rpm
1rad/s = 9.549rpm
*/

speedRampData srd;

void HMotorPinCfg(void)
{
	srd.run_state = STOP;
	srd.dir = CW;
	srd.step_delay = 0;
	srd.decel_start = 0;
	srd.decel_val = 0;
	srd.min_delay = 0;
	srd.accel_count = 0;

	// 清零运动计数与加减速中间变量
	srd.step_count = 0;
	srd.last_accel_delay = 0;
	srd.tempi = 0;
	srd.rest = 0;
	
	GPIO_InitTypeDef          GPIO_Inits;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	
	GPIO_Inits.GPIO_Pin = HostMotorPULPin;
	GPIO_Inits.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Inits.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_Inits);
	
	GPIO_Inits.GPIO_Pin = HostMotorENPin;
	GPIO_Inits.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Inits.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOB, &GPIO_Inits);
	HostMotorOffLine();
	
	GPIO_Inits.GPIO_Pin = HostMotorDIRPin;
	GPIO_Inits.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Inits.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOB, &GPIO_Inits);
	HostMotorCW();                            //顺时针
}

void HMotorITCfg(void)
{
	NVIC_InitTypeDef 		      NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
	NVIC_Init(&NVIC_InitStructure); 
}

void HMotorPeriphCfg(void)
{
	TIM_TimeBaseInitTypeDef   TIM_TimeBaseStructure;
	TIM_OCInitTypeDef         TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = TIMx_PERIOD;	            //0XFFFF
	TIM_TimeBaseStructure.TIM_Prescaler= (TIMx_PRESCALER-1);	  //4-1=3   3+1 = 4
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
  //定时器2向上计数	
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;	
	TIM_TimeBaseInit(HostMotorPer, &TIM_TimeBaseStructure);
  //计数器的值与比较捕获寄存器的值相同进行翻转
	//翻转模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
  //输出极性低
	/*当定时器计数值小于CCR1_Val时为高电平*/
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OC1Init(HostMotorPer, &TIM_OCInitStructure);
	
	TIM_ClearITPendingBit(HostMotorPer,TIM_IT_CC1);
    TIM_ITConfig(HostMotorPer,TIM_IT_CC1,ENABLE);
	TIM_CtrlPWMOutputs(HostMotorPer, ENABLE);
	HostMotorStop();
}

void StepMotorCfg(void)
{
	HMotorPinCfg();
	HMotorITCfg();
	HMotorPeriphCfg();
}

//方向控制
static void Motor_Dir(u8 TempDir)
{
	if(TempDir) 
		HostMotorCCW();
	else 
		HostMotorCW();
}

/*
  * 函数功能: 相对位置运动：运动给定的步数
  * 输入参数: step： 移动的步数
              accel  加速度,单位0.1*rad/sec^2
              decel  减速度,单位0.1*rad/sec^2
              speed  最大速度单位实际时0.1rad/sec
*/
extern void ClearAllFlag(void);
void STEPMOTOR_AxisMoveRel( int32_t step, uint32_t accel, uint32_t decel, uint32_t speed)
{
	ClearAllFlag();	
	
	u32 max_s_lim;                    //达到最大速度时需要的步数
	u32 accel_lim;                    //必须要开始减速的步数(如果还没加速到达最大速度时)
	if(step == 0) //只允许步进电机在停止的时候才继续
		return;
	if(step < 0)                      //步数为负，逆时针转
	{
		srd.dir = CCW;                       
		step =-step;                    //获取步数绝对值
	}
	else
		srd.dir = CW;                   //正转
	Motor_Dir(srd.dir);               //设置电机方向
	if(step == 1)                     //如果只移动一步               
	{
		srd.run_state = DECEL;          //进入减速阶段
		srd.accel_count = -1;           //减速的步数是1步
		srd.step_delay = 1000;          //给一个默认速度(C值)，去执行这一步
	}
	else if(step != 0)                //如果目标运动步数不为1  且不等于0  即大于1          
	{
		//min_delay = (alpha / tt)/ w
		srd.min_delay = (int32_t)(A_T_x10/speed);                       //计算得到最大速度时需要设置的计数器值
		srd.step_delay = (int32_t)((T1_FREQ_148*sqrt(A_SQ/accel))/10);  //计算第一步需要的计数器计数值
//		#ifdef UartDebug
//		printf("min_delay step_delay  is %d %d \r\n",srd.min_delay,srd.step_delay);
//		#endif			
		max_s_lim = (uint32_t)(speed*speed/(A_x200*accel/10));          //计算达到最大速度时需要的步数
		if(max_s_lim == 0)                                              //如果计算出来是个小数(取整)，我最少需要给一整步
			max_s_lim = 1;                                                //给一整步

		accel_lim = (uint32_t)(step*decel/(accel+decel));               //计算多少步之后我们必须开始减速
		if(accel_lim == 0)                                              //如果计算出来是个小数(取整)，我最少需要给一整步
			accel_lim = 1;                                                //给一整步
		if(accel_lim <= max_s_lim)                                      //如果需要减速的步数 小于等于 达到最大速度时的步数  则是三角形
			srd.decel_val = accel_lim - step;                             //减速段的步数 = 总步数 减去 加速段的步数（负数）
		else                                                            //如果需要减速的步数 大于 达到最大速度时的步数  则是梯形
			srd.decel_val = -(max_s_lim*accel/decel);                     //依据公式 n1*W1 = n2*W2 求出减速段的步数
		if(srd.decel_val == 0)                                          //如果计算出来是个小数(取整)，我最少需要给一整步
			srd.decel_val = -1;                                           //给一整步
		srd.decel_start = step + srd.decel_val;                         //计算开始减速时的步数值  总步数-减速段的步数
		if(srd.step_delay <= srd.min_delay)                             //当前转速大于最高转速
		{
			srd.step_delay = srd.min_delay;                               //保持最高速运行
			srd.run_state = RUN;                                          //切换到匀速状态
//			#ifdef UartDebug
//			printf("in_SUN \r\n");
//			#endif				
		}
		else                                                            //速度还没有到达最高速 则继续加速
		{
			srd.run_state = ACCEL;                                        //进入加速状态
			srd.last_accel_delay = srd.min_delay; // 三角形曲线减速起点兜底
//			#ifdef UartDebug
//			printf("in_ACCEL \r\n");
//			#endif	
		}			
		srd.accel_count = 0;                                            //复位加速度计数值
	}
	int timer_count=TIM_GetCounter(HostMotorPer);                             //读一下目前计数器的计数值	
	TIM_SetCompare1(HostMotorPer,timer_count+srd.step_delay/2);       //在这个计数值的基础上加上算出来的计数值，设置给定时器
  HostMotorStart();
}
	
extern void ClearAllFlag(void);
void TIM2_IRQHandler (void)
{
	uint32_t tim_count=0;
	uint32_t tmp = 0;
	uint16_t new_step_delay=0;              //保存下一个延时周期
                                 					//定时器使用翻转模式，需要进入两次中断才输出一个完整脉冲
  //发生输出比较中断
	if(TIM_GetITStatus(HostMotorPer,TIM_IT_CC1)!=RESET)
	{
		//清除中断标志
		TIM_ClearITPendingBit(HostMotorPer,TIM_IT_CC1);
		tim_count=TIM_GetCounter(HostMotorPer); //读出当前计数器的计数值
		tmp = tim_count+srd.step_delay/2;       //求出下一次需要中断的值
		TIM_SetCompare1(HostMotorPer,tmp);      //设置下一次需要中断的值
		
	  srd.tempi++;                                //定时器中断次数计数值
		if(srd.tempi==2) 	                          //2次，说明已经输出一个完整脉冲	
		{
			srd.tempi=0;                              //清零定时器中断次数计数值 			
			switch(srd.run_state)                 //进入状态机
			{
				/*步进电机停止状态*/
				case STOP:
				{
					ClearAllFlag();
					srd.step_count = 0;                   //清零步数计数器	
					srd.rest = 0;                         //清零剩余步数值			  
				}break;
				/*步进电机加速状态*/
				case ACCEL:
				{
					srd.step_count++;                                                                                    //计数器++
					srd.accel_count++;                                                                               //加减速速模式的计数器++
					new_step_delay = srd.step_delay - (((2 *srd.step_delay) + srd.rest)/(4 * srd.accel_count + 1));      //计算下一个脉冲时间间隔 
					srd.rest = ((2 * srd.step_delay)+srd.rest)%(4 * srd.accel_count + 1);                                    // 计算余数，下次计算补上余数，减少误差
					if(srd.step_count >= srd.decel_start)                                                                //如果记录的步数大于减速应该开始的步数，进入减速状态  
					{
						srd.accel_count = srd.decel_val;                                                               //减速的脉冲数(负值)  付过去
						srd.run_state = DECEL;             
					}
					else if(new_step_delay <= srd.min_delay)                                                         //进入匀速了
					{
						srd.last_accel_delay = new_step_delay;      //保存加速过程中最后一次延时 脉冲周期
						new_step_delay = srd.min_delay;         //匀速时下一步的延时 就是min_delay 
						srd.rest = 0; 
						srd.run_state = RUN;
					}				
				}break;
        /*匀速运行*/
				case RUN:
				{
					srd.step_count++; 
					new_step_delay = srd.min_delay; 
					if(srd.step_count >= srd.decel_start)         //从匀速到减速的情况
					{
						srd.accel_count = srd.decel_val;
						new_step_delay = srd.last_accel_delay;      //加阶段最后的延时做为减速阶段的起始延时(脉冲周期)
						srd.run_state = DECEL;
					}			
				}break;
        /*减速*/
				case DECEL:
				{
					srd.step_count++;
					srd.accel_count++;                        //减速的脉冲数++
					new_step_delay = srd.step_delay - (((2 * srd.step_delay) + srd.rest)/(4 * srd.accel_count + 1));
					srd.rest = ((2 * srd.step_delay)+srd.rest)%(4 * srd.accel_count + 1);		  
					if(srd.accel_count >= 0)                  //减速
					{
						srd.run_state = STOP;
            HostMotorStop(); // 走完直接关定时器，不再输出多余脉冲
					}						
				}break;
			}      
			srd.step_delay = new_step_delay;              // 为下个(新的)延时(脉冲周期)赋值
		}
	}
}





