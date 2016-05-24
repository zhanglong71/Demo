
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "driver.h"

#include "l_oled.h"
#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "l_i2c_datetime.h"
#include "l_queryRecord.h"
#include "f_set.h"
#include "f_pswd.h"
#include "f_finger.h"
#include "f_idle.h"
#include "f_bluetooth.h"
#include "f_mngpsword.h"
#include "f_LowPowerAlarm.h"

/*******************************************************************************
 * bluetooth function
 *
 * 进入此的3种可能场景
 *	1.休眠(联接上了)
 	2.待机
 *	3.输入密码(似乎不合理)
 *
 * 
 *******************************************************************************/
int f_bluetooth(unsigned *pMsg)
{
	func_t func;
    u8  u8Ret;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        promptInit();
    	promptDelay(TIMER_100MS); 	
		
    	//开锁
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	vp_stor(CVOPID_Please);
    	vp_stor(CVOPID_TURNHANDLE);
    	
		lockAction();					/** 开锁 **/
		
    	OLED_cleanPages(0, 7);			//清屏
		//DISP_line2nd_PSword_ID(g_u8pswordSel);
		DISP_line2nd_Bluetooth_ID(g_u8pswordSel);
		DISP_PSverify_line3rd_succ(1);	
				
		/** 记录此开锁事件 **/
		MRECORD_setRecordHead(x);
		MRECORD_setRecordLen(x);
		Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分"));	//时间写到数据区
		MRECORD_cleanRecordFlag(x);
		MRECORD_setRecordBTid(g_u8pswordSel);
		record_write(recordIdxTab_getFreeRecordId());
		/** record table **/		
		recordIdxTab_update();
		
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	break;
    	
	case CMSG_BTMATCH:
	case CMSG_BTENROLL:
		u8Ret = bluetooth_Query(g_u8bluetooth);
		if(u8Ret > 0) {
			g_u8pswordSel = u8Ret;
			promptInit();
			
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		} else {
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		}
		
		break;
	case CMSG_TMR:
 		g_tick++;
		if(g_tick > 10) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		
			fstack_init(&g_fstack);
    		func.func = f_LowPowerAlarm;
    		fstack_push(&g_fstack, &func);
		} else {
			if(BLE_LINKED("check BLE status")) {
				/** do nothing **/
			} else {
				/** bluetooth disconnected **/
				fstack_init(&g_fstack);
				func.func = f_idle;
    			fstack_push(&g_fstack, &func);
    			/** goto sleep as soon as possible **/
    			g_tick = 10;
				SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			}
		}
		break;	
		
	case CMSG_BLELINK:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		break;
			
	case CPMT_OVER:		/** 提示音完成 **/	
		promptInit();
		break;
	
	case CACT_OVER:		/** 开锁完成 **/	
		actionInit();
			
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
	
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
    		
    	default:					/** 合适吗？ **/
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	  
    		fstack_init(&g_fstack);
			func.func = f_pswd;
    		fstack_push(&g_fstack, &func);
			break;
    	}
        break;	
		
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;	
}

/////////////////////////////////////////////////////

