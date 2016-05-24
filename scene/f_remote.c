
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
 * ң�ز���
 *******************************************************************************/
int f_remote(unsigned *pMsg)
{
    func_t func;
    
    switch(((msg_t *)pMsg)->msgType)
	{	
	case CMSG_TMR:		/** �͵籨����ʾ��ʱ **/
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
    	promptDelay(TIMER_200MS);	/** ���ڴ˼�����ʾ���������Ҫ(��ΪNY3P087��,����TIMER_100MS������������ԭ���� **/
		break;

	case CPMT_OVER:					/** ��ʾ������������ **/
    	promptInit();
    	lockAction();				/** ���� **/
		break;
			
	case CACT_OVER:					/** ����������̽��� **/
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
