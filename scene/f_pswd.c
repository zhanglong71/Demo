
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
#include "l_i2c_datetime.h"
#include "l_queryRecord.h"
#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "f_pswd.h"
#include "f_finger.h"
#include "f_idle.h"
#include "f_bluetooth.h"
#include "f_mngpsword.h"
#include "f_LowPowerAlarm.h"

u8 g_u8pswordSel;	/** 录密码时，选中要录入的密码的编号(第几条密码) **/
u8 g_u8pswordNO;	/** 录密码时，记录是第几次录入的密码(第几次录入的密码，也就是按下#号的次数) **/
u8 g_u8pswordLen;	/** 录密码时，记录密码的长度(按下#号后，记录先前录入的长度) **/
/*******************************************************************************
 * g_u8password[] 多种用途
 * 1. 密码输入
 * 2. 密码录入及确认
 * 3. PCF8563时间设置
 * 4. 开锁记录查询
 *******************************************************************************/
u8 g_u8password[CPSWORD_MAXLEN * 2];	//两个最大长度密码的空间(注: 1.低电校验重用了此空间。 2. PCF8563时间设置使用此空间作为数据缓冲)

/*******************************************************************************
 * password function
 *******************************************************************************/
int f_pswd(unsigned *pMsg)
{
    func_t func;
    //msg_t	msg;
    //u8	u8Ret;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CACT_OVER:
        //lockAction();
		break;

	case CPMT_OVER:
		promptInit();
		break;
	
	case CMSG_INIT:		/** 待机状态进入密码比对状态 **/
        MFPid_CLEAN("清空FPid");     
		goto	CGOTO_PSWD_FUNC;
	case CMSG_SECO:		//指纹验证通过后,进入密码比对状态(与直接进入密码开锁状态的不同之处：不清空FPid)
		
	  CGOTO_PSWD_FUNC:
        /** bluetooth **/
        bluetooth_PWRon();
        bluetooth_ATdisable();
        
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	adcSample_Start(&g_adcData);
		
        MPSWORD_CLEAN(x);
        All_Screen(0);
        DISP_line1st_psword();
        DISP_line2nd_psword();
        DISP_line3rd_psword(0);
        
		vp_stor(CVOPID_Please);
		vp_stor(CVOPID_INPUT);
		vp_stor(CVOPID_PSWORD);
		promptDelay(TIMER_500MS);
		vp_stor(CVOPID_Press);
		vp_stor(CVOPID_pound);
		vp_stor(CVOPID_Hao);
		vp_stor(CVOPID_OVER);
		break;

	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		if(++g_tick > 20) {
			fstack_init(&g_fstack);
			func.func = f_LowPowerAlarm;
    		fstack_push(&g_fstack, &func);
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		
		#if	0
		if(g_tick < 3) {
			BLUETOOTHPort_Init();
		}
        bluetooth_PWRon();	//???????????????????????????????????
		#endif
		
		break;
		
			#if	0
	case CMSG_BTMATCH:
	case CMSG_BTENROLL:
		u8Ret = bluetooth_Query(g_u8bluetooth);
		if(u8Ret > 0) {
			g_u8pswordSel = u8Ret;
			promptInit();
			fstack_init(&g_fstack);
			func.func = f_bluetooth;
    		fstack_push(&g_fstack, &func);
            
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		break;
			#endif
			
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:					/** 室内设置 **/
			//ledPrompt(0, TIMER_100MS); 	//for test only
			promptInit();
			beep();
    		break;
    		
    	case CKEY_1:
    	case CKEY_2:
    	case CKEY_3:
    	case CKEY_4:
    	case CKEY_5:
    	case CKEY_6:
    	case CKEY_7:
    	case CKEY_8:
    	case CKEY_9:	
    	case CKEY_0:
    		//g_tick = 0;		//不清零，防止故意的长时间按键
    		ledPrompt(((msg_t *)pMsg)->msgValue, TIMER_100MS); 	//for test only
    		
			promptInit();
    		beep();
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);
			DISP_line3rd_psword(MPSWORD_GETLEN("password"));
			break;
				
    	case CKEY_asterisk:
    		if(MPSWORD_GETLEN(x) == 0) {
				promptInit();
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_LowPowerAlarm;
    			fstack_push(&g_fstack, &func);
    		} else {
    			ledPrompt(10, TIMER_100MS); 	//for test only
    			promptInit();
    			beep();
    		
				MPSWORD_POP(((msg_t *)pMsg)->msgValue);
				DISP_line3rd_psword(MPSWORD_GETLEN("password"));
			}
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
			promptInit();
    		beep();
    		
    		//bluetooth_PWRoff();			//关闭可能的蓝牙模块电源
        	//bluetooth_ATdisable();
        
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		#if	1
    		//g_u8pswordSel = psword_Query(g_u8password);
    		//g_u8pswordSel = psword_Query_00(g_u8password);
			//g_u8pswordSel = psword_QueryTab[0](g_u8password)
			
    		if(isMmgPswdEnable()) {	//启用虚位密码功能了吗?
    			g_u8pswordSel = psword_Query_11(g_u8password);
    		} else {
    			g_u8pswordSel = psword_Query_00(g_u8password);
    		}
			if(g_u8pswordSel != 0) {
    			fstack_init(&g_fstack);
				func.func = f_pswd_succ;
    			fstack_push(&g_fstack, &func);
			} else {
				fstack_init(&g_fstack);
				func.func = f_pswd_fail;
    			fstack_push(&g_fstack, &func);
			}
			#endif
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

int f_pswd_fail(unsigned *pMsg)
{
	func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
       	g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT消息 **/
		
		promptInit();	
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_FAILED);
    	promptDelay(TIMER_100MS);
    	
    	OLED_cleanPages(0, 7);	//清除
    	DISP_PSverify_line3rd_succ(0);
		break;	
	
	case CMSG_TMR:    
		g_tick++;
		if(g_tick > 10) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
			fstack_init(&g_fstack);
			func.func = f_LowPowerAlarm;
    		fstack_push(&g_fstack, &func);
		}
		break;	
		
	case CPMT_OVER:		/** 提示音完成 **/	
		promptInit();
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
					
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;	
}

int f_pswd_succ(unsigned *pMsg)
{
	func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        promptInit();
    	promptDelay(TIMER_100MS); 	
    	
    	if(isOpenMode_FPorPS()) {	//"指纹/密码"模式
		/**
		 * 开锁的场景1："指纹/密码"模式，指纹验证通过
		 **/
		 /** 记录此开锁事件 **/
			MRECORD_setRecordHead(x);
			MRECORD_setRecordLen(x);
			Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
			MRECORD_cleanRecordFlag(x);
			MRECORD_setRecordPWid(g_u8pswordSel);
			record_write(recordIdxTab_getFreeRecordId());
			/** record table **/
			recordIdxTab_update();
		CGOTO_OPENLOCK:					//开锁
			
			vp_stor(CVOPID_COMPARISON);
    		vp_stor(CVOPID_PASSED);
    		vp_stor(CVOPID_Please);
    		vp_stor(CVOPID_TURNHANDLE);
    		
			lockAction();				/** 开锁 **/
			
    		OLED_cleanPages(0, 7);			//清屏
			DISP_line2nd_PSword_ID(g_u8pswordSel);
			DISP_PSverify_line3rd_succ(1);	
			
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		break;
    	} else if(isOpenMode_FPandPS()) {	//"指纹+密码"模式
			//if((MFPid_GETLEN("指纹比对过一次")) && (MFPid_GETid("指纹比对有效"))) {
			if(MFPid_GETLEN("指纹比对过一次")) {
				 /** 记录此开锁事件 **/
				MRECORD_setRecordHead(x);
				MRECORD_setRecordLen(x);
				Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
				MRECORD_cleanRecordFlag(x);
				MRECORD_setRecordFPid(MFPid_TOP(0));
				MRECORD_setRecordPWid(g_u8pswordSel);
				record_write(recordIdxTab_getFreeRecordId());
				/** record table **/
				recordIdxTab_update();
				goto	CGOTO_OPENLOCK;	
			} else {	
				/***************************************************************
				 * (指纹+密码)模式，缺少指纹比对. 应该跳转到验证指纹的流程中去
				 * 
				 ***************************************************************/
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
				fstack_init(&g_fstack);
				func.func = f_pswd_fail;
    			fstack_push(&g_fstack, &func);
			}
    	} else {		
    		//异常处理(既不是指纹+密码，又不是指纹/密码，那又是什么模式呢?)
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_pswd_fail;
    		fstack_push(&g_fstack, &func);
    	}
    	
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	vp_stor(CVOPID_Please);
    	vp_stor(CVOPID_TURNHANDLE);
    	
		lockAction();	/** 开锁 **/
		
    	OLED_cleanPages(0, 7);			//清屏
		DISP_line2nd_PSword_ID(g_u8pswordSel);
		DISP_PSverify_line3rd_succ(1);	
		
    	#if	1
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	#endif
		break;	
	
	case CMSG_TMR:
 		g_tick++;
		if(g_tick > 10) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		
			fstack_init(&g_fstack);
    		func.func = f_LowPowerAlarm;
    		fstack_push(&g_fstack, &func);
		}
		break;	
		
	case CPMT_OVER:		/** 提示音完成 **/	
		promptInit();
		break;
		
	case CACT_OVER:		/** 开锁前的提示音完成 **/	
		actionInit();
			
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;	
}
/////////////////////////////////////////////////////

