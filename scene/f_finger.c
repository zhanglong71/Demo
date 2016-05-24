
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
#include "f_idle.h"
#include "f_pswd.h"
#include "f_poweron.h"
#include "f_finger.h"
#include "f_LowPowerAlarm.h"

u16 g_u16FPid[3];	//长度+第一次+第二次

static u8 u8SearchCount = 0;	/** 比对次数 **/
/*******************************************************************************
 * local function
 * function: 开启电源，等待供电稳定
 * 
 *******************************************************************************/
int f_finger(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:
		if(g_tick++ >= 5) {		//语音播放超时
    		fstack_init(&g_fstack);
			func.func = f_fingerError;
    		fstack_push(&g_fstack, &func);
		} else {
			//goto	GOTO_FPOWERON;
		}
		break;

	//GOTO_FPOWERON:
	case CMSG_INIT:
		promptInit();     /****/
		if(fingerTab_isEmpty()) {
    		fstack_init(&g_fstack);
			func.func = f_fingerEmpty;
    		fstack_push(&g_fstack, &func);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		} else {
        	MFPOWER_ON();					//指纹传感器供电
        	
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        	promptDelay(TIMER_100MS);		//等待---供电稳定后，向指纹传感器发送初始化命令
        	MFPid_CLEAN("清空FPid");
    	}
    	adcSample_Start(&g_adcData);		//电压采样开始
		break;
		
	case CMSG_SECO:			//再次进入指纹比对状态
		MFPOWER_ON();					//指纹传感器供电
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        //promptDelay(TIMER_100MS);		//等待---供电稳定后，向指纹传感器发送初始化命令
        
    	vp_stor(CVOPID_Please);
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_ANOTHER);
    	vp_stor(CVOPID_FPRINT);
    	
    	//vp_stor(CVOPID_Press);
    	//vp_stor(CVOPID_FINGER);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_init(&g_fstack);
		func.func = f_fingerInit;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:
		break;
	}  

    return  0;
}
/*******************************************************************************
 * finger initial command and response
 * 初始化
 *******************************************************************************/
int f_fingerInit(unsigned *pMsg)
{
  	//int i;
    func_t func;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_TMR:
		if(g_tick++ >= 3) {		//3s内初始化失败(指纹传感器故障)
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_init(&g_fstack);
			func.func = f_fingerError;
    		fstack_push(&g_fstack, &func);
    		
        	MFPOWER_OFF();
		} else {
			FP_INIT();
		}
		//ledPrompt(1, TIMER_100MS);
		break;
		
	case CMSG_INIT:
        FP_INIT();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        //ledPrompt(2, TIMER_100MS);
		break;
		
	case CMSG_FGINIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
        //ledPrompt(3, TIMER_100MS);
		
		fstack_init(&g_fstack);
		func.func = f_fingerSearch;
    	fstack_push(&g_fstack, &func);
		break;
	case CFIGER_ON:		/** 检测到指纹按下 **/
        LEDall_off();
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
			
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * finger
 * Description: device error
 *******************************************************************************/
int f_fingerError(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:    
		g_tick++;
        //ledPrompt(12, TIMER_100MS);
		break;

	case CMSG_INIT:
        MFPOWER_OFF();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
		fstack_init(&g_fstack);
		func.func = f_idle;
    	fstack_push(&g_fstack, &func);	
		break;
		
	case CFIGER_ON:		/** 检测到指纹按下 **/
        //ledPrompt(11, TIMER_100MS);
		break;
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * finger search
 * Description: 执行查询动作
 *******************************************************************************/
int f_fingerSearch(unsigned *pMsg)
{
    func_t func;
    //int i;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:    
		g_tick++;
		if(g_tick > 10)	/** 超时 **/
		{
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			fstack_init(&g_fstack);
			func.func = f_fingerSearchFail;
    		fstack_push(&g_fstack, &func);
		}
		break;

	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		FP_Query(0, MUSER_ID_MAX - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		//FP_Query(0, CTOTALFINGER - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
        u8SearchCount = 0;
		break;
		
	case CMSG_FGQUERY:
		/** 验证通过 **/
		MFPOWER_OFF();					/** 指纹头断电 **/
		g_tick = 0;						/** 防止可能的操作途中，超时退出 **/
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	/** 定时 **/
    	
    	fstack_init(&g_fstack);
		func.func = f_fingerSearchSuccess;
    	fstack_push(&g_fstack, &func);
		break;	
		
	case CMSG_FGDOING:					/** 命令数据错 **/
	case CMSG_FGQUERYING:				/** 动作进行中, 或图像质量不好 **/
	case CMSG_FGCOMFAIL:				/** 通信校验错。可以重试 **/
		FP_Query(0, MUSER_ID_MAX - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		//FP_Query(0, CTOTALFINGER - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		break;
		
	case CMSG_FGOPFAIL:
		if(++u8SearchCount < 3) {
			FP_Query(0, MUSER_ID_MAX - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
			//FP_Query(0, CTOTALFINGER - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		} else {
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	/** 定时 **/
			fstack_init(&g_fstack);
			func.func = f_fingerSearchFail;
    		fstack_push(&g_fstack, &func);
		}
		break;
		
	case CFIGER_ON:		/** 检测到指纹按下 **/
        //LEDall_off();
		break;
			
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * f_fingerSearchSuccess
 * Description: 查询成功。判断是否要执行开锁动作
 * 
 * 需要考虑的场景
 * 场景1："指纹/密码"模式(开锁)
 * 场景2："指纹+指纹"模式(只有一枚指纹：(Display + VOP)开锁)
 * 场景2："指纹+指纹"模式(多于一枚指纹：(Display + VOP)然后等待进入下一轮验证)
 * 场景3："指纹+指纹"模式(两枚指纹都通过，两枚指纹不同：(Display + VOP)开锁)
 * 场景4："指纹+指纹"模式(两枚指纹都通过，两枚指纹相同：(Display + VOP)验证失败)
 * 场景5："指纹+密码"模式(无密码：(Display + VOP)开锁)
 * 场景6："指纹+密码"模式(有密码：(Display + VOP)等待进入下一轮验证)
 * 
 * CMSG_INIT
 * 	1. 显示通过验证的身份
 * 	2. 语音提示：验证通过
 * 	3. 如果满足开锁条件，执行开锁动作
 * 
 * CPMT_OVER
 * 语音提示完成后，执行下步动作：
 *	1. 如果满足开锁条件，表明已经开锁了，不需要做任何事情(滤过)
 *  2. 如果在等待下一轮身份认证，直接进入下一轮。注意发适当的消息
 *******************************************************************************/
int f_fingerSearchSuccess(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
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

	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT消息 **/
		/** voicePrompt **/
		promptInit();
    	promptDelay(TIMER_100MS);
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	/** OLED Display **/
    	OLED_cleanPages(0, 7);			//清屏
        if(MisADMIN(MFPid_TOP(0))) {
        	DISP_line2nd_PFadmin_ID(M2AdminID(MFPid_TOP(0)));
		} else {
			DISP_line2nd_PFuser_ID(M2UserID(MFPid_TOP(0)));
		}
		DISP_PSverify_line3rd_succ(1);
		
		/** 查是否达到开锁条件? **/
		if(isOpenMode_FPorPS()) {	//"指纹/密码"模式
			/**
			 * 开锁的场景1："指纹/密码"模式，指纹验证通过
			 **/
		 
			/** 记录此开锁事件 **/
			MRECORD_setRecordHead(x);
			MRECORD_setRecordLen(x);
			Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
			MRECORD_cleanRecordFlag(x);
			MRECORD_setRecordFPid(MFPid_TOP(0));
			record_write(recordIdxTab_getFreeRecordId());
			/** record table **/
			recordIdxTab_update();
		 
	CGOTO_OPENLOCK:					//开锁
			
    		vp_stor(CVOPID_Please);
    		vp_stor(CVOPID_TURNHANDLE);
			lockAction();	/** 开锁 **/
    		break;
    	} else if(isOpenMode_FPandFP()) {	//"指纹+指纹"模式
    		if((fingerTab_getBusyEntryNum() < 2)) {
    			/**
				 * 开锁的场景2："指纹+指纹"模式，仅有一枚指纹，且验证通过
			 	 **/
			 	 
				/** 记录此开锁事件 **/
				MRECORD_setRecordHead(x);
				MRECORD_setRecordLen(x);
				Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
				MRECORD_cleanRecordFlag(x);
				MRECORD_setRecordFPid(MFPid_TOP(0));
				record_write(recordIdxTab_getFreeRecordId());
				/** record table **/
				recordIdxTab_update();
				
				goto	CGOTO_OPENLOCK;
    		} else {	//已录入过多枚指纹
    			if(MFPid_GETLEN("通过了指纹比对的个数") >= 2) {
    				if(MisFPid_NEQ("两次比对的是不同的指纹")) {
    					/**
			 			 * 开锁的场景3："指纹+指纹"模式，两枚不同指纹验证通过
			 			 **/
			 			/** 记录此开锁事件 **/
						MRECORD_setRecordHead(x);
						MRECORD_setRecordLen(x);
						Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
						MRECORD_cleanRecordFlag(x);
						MRECORD_setRecordFPid(MFPid_TOP(1));
						MRECORD_setRecord2ndFPid(MFPid_TOP(0));
						record_write(recordIdxTab_getFreeRecordId());
						/** record table **/
						recordIdxTab_update();
				
						goto	CGOTO_OPENLOCK;
					} else {	//两次比对的指纹相同
						g_tick = 0;
						SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
						fstack_init(&g_fstack);
						func.func = f_fingerSearchFail;
    					fstack_push(&g_fstack, &func);
					}
    			} else {	//只比对了一枚指纹，再比对下一枚指纹(播完语音后在CPMT_OVER中处理)
    			}
    		}
    	} if(isOpenMode_FPandPS()) {	//"指纹+密码"模式
    		if(psword_getBusyEntryNum() <= 0) {
    			/**
			 	 * 开锁的场景4："指纹+密码"模式，没有录入密码
			 	 **/
			 	 /** 记录此开锁事件 **/
					MRECORD_setRecordHead(x);
					MRECORD_setRecordLen(x);
					Mdatetime_read(MRECORD_getDataAddr("取数据正式开始的部分")); 	//时间写到数据区
					MRECORD_cleanRecordFlag(x);
					MRECORD_setRecordFPid(MFPid_TOP(0));
					record_write(recordIdxTab_getFreeRecordId());
					/** record table **/
					recordIdxTab_update();
				
				goto	CGOTO_OPENLOCK;
    		} else {	//在播放完语音后，再进入密码验证
    			#if	0
    			promptInit();
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	    	
    			fstack_init(&g_fstack);
				func.func = f_pswd;
    			fstack_push(&g_fstack, &func);
    			#endif
    		}
    	}
		break;
	
	case CPMT_OVER:		/** 开锁前的提示音完成 **/	
		promptInit();
		/** 查是否达到开锁条件? **/
		if(isOpenMode_FPorPS()) {	//"指纹/密码"模式
		/**
		 * 开锁的场景1：此处无动作
		 **/
    	} else if(isOpenMode_FPandFP()) {	//"指纹+指纹"模式
    		if((fingerTab_getBusyEntryNum() < 2)) {
    			/**
				 * 开锁的场景2："指纹+指纹"模式，仅有一枚指纹，且验证通过(无动作)
			 	 **/
    		} else {	//已录入过多枚指纹
    			if(MFPid_GETLEN("通过了指纹比对的个数") >= 2) {
    				if(MisFPid_NEQ("两次比对的是不同的指纹")) {
    					/**
			 			 * 开锁的场景3："指纹+指纹"模式，两枚不同指纹验证通过(无动作)
			 			 **/
					} else {	//两次比对的指纹相同(不会执行到这里)
						#if	0
						g_tick = 0;
						SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
						fstack_init(&g_fstack);
						func.func = f_fingerSearchFail;
    					fstack_push(&g_fstack, &func);
    					#endif
					}
    			} else {	//只比对了一枚指纹，再比对下一枚指纹
					fstack_init(&g_fstack);
    				func.func = f_finger;
    				fstack_push(&g_fstack, &func);
    				
					g_tick = 0;	
					SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_SECO);
    			}
    		}
    	} if(isOpenMode_FPandPS()) {	//"指纹+密码"模式
    		if(psword_getBusyEntryNum() > 0) {
				promptInit();
    			g_tick = 0;
    			//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_SECO);
    	    	
    			fstack_init(&g_fstack);
				func.func = f_pswd;
    			fstack_push(&g_fstack, &func);
    		} else {
    			/**
			 	 * 开锁的场景4："指纹+密码"模式，没有录入密码(无动作)
			 	 **/
    		}
    	}
		break;

	case CACT_OVER:		/** 开锁完成 **/	
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

    return  0;
}


/*******************************************************************************
 * f_fingerSearchFail
 * Description: 查询失败，报警后退出
 *******************************************************************************/
int f_fingerSearchFail(unsigned *pMsg)
{
	func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
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
	
	case CPMT_OVER:		/** 提示音完成 **/	
		promptInit();
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
			
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * f_fingerEmpty
 * Description: 无指纹模板
 *******************************************************************************/
int f_fingerEmpty(unsigned *pMsg)
{
	func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:    
		g_tick++;
		if(g_tick > 10) {
			g_tick = 0;	
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
			fstack_init(&g_fstack);
    		func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}		
		break;

	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT消息 **/
		
		promptInit();	
		vp_stor(CVOPID_FORYOURSAFTY);	//为了您的安全
    	vp_stor(CVOPID_PLEASEPRESS);	//请按
    	vp_stor(CVOPID_SET);			//设置
    	vp_stor(CVOPID_Key);			//键
    	vp_stor(CVOPID_ADD);			//添加
    	vp_stor(CVOPID_MANAGER);		//管理员
    	vp_stor(CVOPID_FPRINT);			//指纹
    	
    	OLED_cleanPages(0, 0);	//清除
    	OLED_cleanPages(7, 7);	//清除
    	//DISP_PSverify_line3rd_succ(0);
    	DISP_open_FPempty();
    	
		lockAction();	/** 开锁 **/
		break;
	
	case CPMT_OVER:		/** 开锁前的提示音完成 **/	
		promptInit();
		break;

	case CACT_OVER:		/** 开锁前的提示音完成 **/	
		actionInit();	
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_idle;
    	fstack_push(&g_fstack, &func);
		break;
			
	default:
		break;
	}  

    return  0;
}
/////////////////////////////////////////////////////

