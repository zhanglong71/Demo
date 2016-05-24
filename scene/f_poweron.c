#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "driver.h"

#include "l_voice.h"
#include "l_oled.h"
#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "f_idle.h"
#include "f_poweron.h"

/*******************************************************************************
 * password function
 *******************************************************************************/
int f_poweron(unsigned *pMsg)
{
    func_t func;
    //int i;
    //msg_t	msg;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CACT_OVER:
		actionInit();	
		g_tick = 10;
		break;

	case CMSG_TMR:
        ledPrompt(0, TIMER_200MS); 	//for test only
		g_tick++;  
		if(g_tick > 10) {
			actionInit();	
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
       		//for(i = 0; i < 12; i++)led_off(i);
       		LEDall_off();
       		//OLEDInit_IC();
		
    		fstack_init(&g_fstack);
			func.func = f_idle;    		
			fstack_push(&g_fstack, &func);
			
			#if	0
			vp_play(CVOPID_RESET);
            Halt_OffDevice();
			halt();
			Halt_OnDevice();
			vp_stor(CVOPID_RESET);
        	mpr121_init();
        	#endif
		}
		break;
		
	case CMSG_INIT:		/** 初始化外设 **/
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	
        mpr121_init();
        OLEDReset_IC();    
       	OLEDInit_IC();
       	OLED_cleanPages(0, 0);
       	OLED_cleanPages(7, 7);
		DISP_PowerOn();
		
    	vp_stor(CVOPID_WELCOME);
    	
    	if(fingerTab_isEmpty()) {	//只有指纹空才有开锁动作
        	lockAction();
		}
		break;
	
	case CPMT_OVER:	
		promptInit();
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_GOOUT:				/** 摸室内把手，直接开锁 **/
    		break;
    		
		case CKEY_SET:				/** 室内设置 **/
    		break;
		
		default:					/** 可能的错误 **/
			break;
		}
		break;

	default:
		break;
	}  

    return  0;
}
/////////////////////////////////////////////////////

