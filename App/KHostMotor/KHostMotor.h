#ifndef __KHostMotor__
#define __KHostMotor__

#include "stm32f10x.h"
#include "math.h"

//PA
#define HostMotorPULPin      	   GPIO_Pin_15

//PB
#define HostMotorENPin       	   GPIO_Pin_3
#define HostMotorDIRPin      	   GPIO_Pin_4


#define HostMotorOffLine()       GPIO_SetBits(GPIOB,HostMotorENPin)       
#define HostMotorOnLine()        GPIO_ResetBits(GPIOB,HostMotorENPin)  

#define HostMotorCCW()           GPIO_SetBits(GPIOB,HostMotorDIRPin)       
#define HostMotorCW()            GPIO_ResetBits(GPIOB,HostMotorDIRPin)  

#define HostMotorPer             TIM2

#define HostMotorStop()          {TIM_CCxCmd(HostMotorPer,TIM_Channel_1,TIM_CCx_Disable);TIM_Cmd(HostMotorPer,DISABLE);}                
#define HostMotorStart()         {TIM_CCxCmd(HostMotorPer,TIM_Channel_1,TIM_CCx_Enable);TIM_Cmd(HostMotorPer,ENABLE);}

#define TIMx_PRESCALER           4         //72/((4-1)+1) = 18MHz
#define TIMx_PERIOD              0XFFFF

//步进电机自己用的状态定义
#define STOP                     0 // 加减速曲线状态：停止
#define ACCEL                    1 // 加减速曲线状态：加速阶段
#define DECEL                    2 // 加减速曲线状态：减速阶段
#define RUN                      3 // 加减速曲线状态：匀速阶段

#define FALSE                    0
#define TRUE                     1

#define  CW                      0 // 顺时针
#define CCW                      1 // 逆时针

#define T1_FREQ     (SystemCoreClock/TIMx_PRESCALER) // 18MHz
#define FSPR        200                //步进电机的一圈所需脉冲数
#define MICRO_STEP  32                 //细分器细分数
#define SPR         (FSPR*MICRO_STEP)  //细分后一圈需要的脉冲数

/*数学常数,用于简化计算*/
#define ALPHA       ((float)(2*3.14159/SPR))     //步进电机的步距角(一个脉冲走的角度)     
#define A_T_x10     ((float)(10*ALPHA*T1_FREQ)) 
#define T1_FREQ_148 ((float)((T1_FREQ*0.676)/10))
#define A_SQ        ((float)(2*100000*ALPHA))
#define A_x200      ((float)(200*ALPHA))

/*梯形加减速相关变量*/
typedef struct 
{
	/*当前电机状态*/
	__IO uint8_t  run_state ; 
	/*旋转方向*/
	__IO uint8_t  dir ;    
	/*脉冲间隔*/
	__IO int32_t step_delay;  
	/*减速位置*/
	__IO uint32_t decel_start; 
	/*减速步数*/
	__IO int32_t decel_val;   
	/*最小间隔*/
	__IO int32_t min_delay;   
	/*加速步数*/
	__IO int32_t accel_count; 


    // 新增迁移变量
    uint32_t step_count;
	
    uint16_t last_accel_delay;
	
    uint8_t  tempi;
		
    int32_t  rest;

}speedRampData;

extern speedRampData srd;

void StepMotorCfg(void);
void STEPMOTOR_AxisMoveRel( int32_t step, uint32_t accel, uint32_t decel, uint32_t speed);

#endif



