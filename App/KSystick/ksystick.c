#include "ksystick.h"
u8 task500ms = 0;
u8 task10ms = 0;
u8 task1ms = 0;
void KsystickCfg(void)
{
	if(SysTick_Config(SystemCoreClock/1000))
	{
//		printf("系统滴答定时器配置失败!\r\n");
		while(1);
	}		
}


