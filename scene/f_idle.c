
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "charQue.h"
#include "driver.h"

#include "l_voice.h"
#include "l_oled.h"
#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "l_i2c_datetime.h"
#include "f_pswd.h"
#include "f_set.h"
#include "f_bluetooth.h"
#include "f_poweron.h"
#include "f_finger.h"
#include "f_remote.h"

/*******************************************************************************
 * local function
 *******************************************************************************/
int f_idle(unsigned *pMsg)
{
    func_t func;
    u8	u8Ret;
    //static int testNO;

    switch(((msg_t *)pMsg)->msgType) 
    //switch(msg.msgType)
	{
	case CACT_OVER:
        //lockAction();
		break;

	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
      #if 0
		OLED_ShowNum12x24(0, 5, g_tick, 7, 24);
      #endif
		g_tick++;
		//if(g_tick < 0)g_tick = 0;
		if(g_tick >=3) {
		//if(g_tick > 100) {
			/** low power consumeption **/
		  #if	1
			vp_play(CVOPID_RESET);
            Halt_OffDevice();
			//halt();
			u8Ret = wakeupCheck();
			if(u8Ret == 0) {
				/** awu **/
				Halt_OnAWU();
			} else if(u8Ret == 1) {
				/** FPrint **/
				Halt_OnDevice();
			} else {
				Halt_OnDevice();
				g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			}
            
			vp_stor(CVOPID_SILENT);
        	mpr121_init();

    	  #else
			//adcSample_Start(&g_adcData);
			while(1) {
				LEDall_Toggle();
				nop();	//
			} 
		  #endif
			
		} else {
			MDATETIME_READ("读出时间，存入到g_u8password[]中");
    		//BCD2HEX_bytes(g_u8password, 7);
    		DISP_GetOrSetDateTime(0);
		}
		break;
	
	case CMSG_PWON:		/** only for power on **/
        g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_init(&g_fstack);
		func.func = f_poweron;
    	fstack_push(&g_fstack, &func);
		break;
			
	case CMSG_INIT:		/** nothing to be done **/
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	adcSample_Stop(&g_adcData);
        mpr121_init();
		All_Screen(0);
        DISP_idle();
		break;
	
	case CPMT_OVER:
		promptInit();
    	break;	
    	
	case CMSG_BTMATCH:
	case CMSG_BTENROLL:
		fstack_init(&g_fstack);
		func.func = f_bluetooth;
    	fstack_push(&g_fstack, &func);
            
    	//g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_BTMATCH);
		break;

	case CMSG_RMT:
    	/** 遥控接收 **/
        fstack_init(&g_fstack);
		func.func = f_remote;
    	fstack_push(&g_fstack, &func);
        
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		break;
		
	//case CMSG_BACK:
	case CMSG_UKEY:					/** 检测到按键松开 **/
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:				/** 室内设置 **/
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_set;
    		fstack_push(&g_fstack, &func);
    		break;
            
    	case CKEY_VOP:				/** 语音开关 **/
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_set_vop;
    		fstack_push(&g_fstack, &func);
    		break;
    		
		default:
			break;
    	}
		break;
		
	case CMSG_DKEY:			/** 检测到按键按下 **/
        switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:				/** 室内设置 **/
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		
    		fstack_init(&g_fstack);
			func.func = f_set_longPress;
    		fstack_push(&g_fstack, &func);
    		break;
				
    	case CKEY_asterisk:			/** 星号 **/
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
    		break;	
    	
    	case CKEY_VOP:				/** 语音开关：ignory **/
    		break;	
            
		default:					/** 合适吗？ **/
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	  
    		fstack_init(&g_fstack);
			func.func = f_pswd;
    		fstack_push(&g_fstack, &func);
			break;
		}
        break;
    	
	case CFIGER_ON:		/** 检测到指纹按下 **/
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	  
    	fstack_init(&g_fstack);
		func.func = f_finger;
    	fstack_push(&g_fstack, &func);
		break;
	
	default:
		break;
	}  

    return  0;
}
/////////////////////////////////////////////////////

