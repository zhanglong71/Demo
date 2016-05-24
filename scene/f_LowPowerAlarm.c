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
 * Description: 开锁操作完成后，是否要查低电压报警
 * 低电报警
 *******************************************************************************/
int f_LowPowerAlarm(unsigned *pMsg)
{
//	msg_t msg;
    func_t func;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_TMR:		/** 低电报警提示计时 **/
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
    	
    	if(g_flag & (1 << 9)){		/** 查低电报警 **/
    		/** 报警提示 **/
    		vp_stor(CVOPID_POWERLOW);
    		promptDelay(TIMER_500MS);
    		
    		DISP_lowPower();
    	} else {
    		goto GOTO_ALLOFF;				/** 无报警动作，直接结束 **/
    	}
		break;

	case CPMT_OVER:							/** 锁体操作过程结束 **/
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

