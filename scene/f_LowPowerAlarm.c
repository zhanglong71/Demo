#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "exti.h"
#include "arch.h"
#include "driver.h"

#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "l_oled.h"

#include "f_LowPowerAlarm.h"
#include "f_idle.h"
#include "f_set.h"
#include "f_mngfinger.h"
#include "f_mngPsword.h"
#include "f_mngSystem.h"

/*******************************************************************************
 * Description: ����������ɺ��Ƿ�Ҫ��͵�ѹ����
 * �͵籨��
 *******************************************************************************/
int f_LowPowerAlarm(unsigned *pMsg)
{
//	msg_t msg;
    func_t func;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_TMR:		/** �͵籨����ʾ��ʱ **/
		g_tick++;
		if(g_tick >= 10)
		{
		GOTO_ALLOFF:
    		promptInit();
    		
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
    		
    		g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			//SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
			
    		All_Screen(0);
        	DISP_idle();
			/*******************************************************************/
		}
		break;
		
	case CMSG_INIT:
    	promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
    	if(g_flag & (1 << 9)){		/** ��͵籨�� **/
    		/** ������ʾ **/
    		vp_stor(CVOPID_POWERLOW);
    		promptDelay(TIMER_500MS);
    		
    		DISP_lowPower();
    	} else {
    		goto GOTO_ALLOFF;				/** �ޱ���������ֱ�ӽ��� **/
    	}
		break;

	case CPMT_OVER:							/** ����������̽��� **/
    	promptInit();
    	bbbeep();
    	promptDelay(TIMER_500MS);
		break;
			
	default:
		break;
	}
    return  0;
}
/////////////////////////////////////////////////////

