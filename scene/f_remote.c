
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "driver.h"

#include "f_LowPowerAlarm.h"

/*******************************************************************************
 * 遥控操作
 *******************************************************************************/
int f_remote(unsigned *pMsg)
{
    func_t func;
    
    switch(((msg_t *)pMsg)->msgType)
	{	
	case CMSG_TMR:		/** 低电报警提示计时 **/
		g_tick++;
		if(g_tick > 10)
		{
			g_tick = 0;	
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
			fstack_init(&g_fstack);
    		func.func = f_LowPowerAlarm;
    		fstack_push(&g_fstack, &func);
    		
    		goto CGOTO_OVER;
		}
		break;
		
	case CMSG_INIT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	beep();
    	promptDelay(TIMER_200MS);	/** 可在此加入提示音，如果需要(改为NY3P087后,参数TIMER_100MS工作不正常，原因不明 **/
		break;

	case CPMT_OVER:					/** 提示音结束，开锁 **/
    	promptInit();
    	lockAction();				/** 开锁 **/
		break;
			
	case CACT_OVER:					/** 锁体操作过程结束 **/
	CGOTO_OVER:
    	actionInit();
    	g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
			
	default:
		break;
	}
	return 0;
}
