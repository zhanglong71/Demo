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

#include "l_queryRecord.h"
#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "l_oled.h"
#include "l_voice.h"

#include "f_idle.h"
#include "f_set.h"
#include "f_finger.h"
#include "f_mngfinger.h"
#include "f_mngPsword.h"
#include "f_mngBluetooth.h"
#include "f_mngSystem.h"
#include "f_mngQuery.h"
#include "f_LowPowerAlarm.h"

u8 g_u8menuNO; 

/** 功能表。注意其中要与显示顺序一致， 以免造成混乱 **/
const static pfunc_t menuTab[] = {
	f_mngFinger,
	f_mngPsword,
	f_mngBluetooth,
	f_mngSystem,
	f_mngQuery,
};
/*******************************************************************************
 * 进入设置菜单。
 * 进行身份认证或录入第1管理员指纹
 *******************************************************************************/
int f_set(unsigned *pMsg)
{	
    func_t func;
    //msg_t	msg;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CPMT_OVER:
		promptInit();
		break;

	case CMSG_INIT:	
		if(fingerTab_isEmpty()) {	//没有指纹模板, 录入指纹模板
    		fstack_init(&g_fstack);
			func.func = f_set_addSuperAdmin;
    		fstack_push(&g_fstack, &func);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		} else {					//有指纹模板，进行身份认证
			
			ClrTimer_irq(&g_timer[0]);
			/** display **/
        	OLED_cleanPages(3, 7);
    		DISP_line1st_authen_manager();	//验证管理员
    		DISP_menu_line2nd_PP(4);		//请按手指
			
			/** voice prompt **/
			vp_stor(CVOPID_COMPARISON);
			vp_stor(CVOPID_MANAGER);
			vp_stor(CVOPID_Please);
			vp_stor(CVOPID_Press);
    		vp_stor(CVOPID_FINGER);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);
			//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_FGINIT);
    		MFPOWER_ON();				//指纹传感器供电
    		
        	MFPid_CLEAN("清空FPid");
    	}
    	MisLOWPOWERCLEAN("清空按键");		//清空按键(设置过程)
		break;
		
	case CMSG_TMR:     
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_MATCH);
    		
			fstack_init(&g_fstack);
			func.func = f_set_authenFail;
    		fstack_push(&g_fstack, &func);
		}
		break;
			
	case CFIGER_INIT:
		FP_INIT();
		break;
	
	//case CMSG_FGCCEL:
	case CMSG_FGINIT:				//完成指纹传感器初始化
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	g_u8pswordNO = 0;
    	FP_Query(0, MUSER_ID_BASE - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		break;
	case CMSG_FGDOING:					/** 命令数据错 **/
	case CMSG_FGQUERYING:				/** 动作进行中 **/
	case CMSG_FGCOMFAIL:				/** 通信校验错。可以重试 **/
		FP_Query(0, MUSER_ID_BASE - 1);	/** 如果有指纹模板，发送指纹比对命令 **/
		break;
		
	case CMSG_FGQUERY:					/** Note: the JP100-A not specified ID **/
		
		if((MFPid_GETid("查id范围") > 0) && (MFPid_GETid("查id范围") <= MUSER_ID_BASE)) {
    		promptInit();
			MFPOWER_OFF();					/** 指纹头断电 **/
			g_tick = 0;						/** 防止可能的操作途中，超时退出 **/
			SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_MATCH);	/** 定时 **/
    		
    		fstack_init(&g_fstack);
			func.func = f_set_authenSucc;
    		fstack_push(&g_fstack, &func);
    	} else {
    		goto CGOTO_AUTHEN_FAILED;
    	}
		break;	
		
	case CMSG_FGOPFAIL:		//超时、没查到、模块空
	CGOTO_AUTHEN_FAILED:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_MATCH);	/** 定时 **/
		
		fstack_init(&g_fstack);
		func.func = f_set_authenFail;
    	fstack_push(&g_fstack, &func);
		break;	
					
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_SET:				/** 室内设置 **/
			break;
			
    	case CKEY_asterisk:
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		beep();
    		MFPOWER_OFF();		//验证权限时，按退出键
    		
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);	//按键入栈，干什么用？进入低电校验、测试模式等状态的操作流程	
    		if(MPSWORD_GETLEN("长度") == 2)
              ;
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
			break;
			
    	case CKEY_3:
    	case CKEY_5:
    	case CKEY_7:
    		promptInit();
    		beep();
    		
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);
			if(MisLOWPOWERCHECK("低电校验") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("恢复出厂设置") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源(why shutdown ?)
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
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

/*******************************************************************************
 * 选择"指纹/密码/系统/查询"功能
 * 管理员身份验证通过/录入第一管理员成功后进到此
 *******************************************************************************/
int f_set_funcSel(unsigned *pMsg)
{
	#define	CMIN	0
	//#define	CMAX	3
	#define	CMAX	(MTABSIZE(menuTab) - 1)
	
    func_t func;
    //msg_t	msg;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		break;

	case CMSG_INIT:
        g_u8menuNO = 0;
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        /** display **/
        OLED_cleanPages(0, 7);
		DISP_line1st_menu();			//第一行
		DISP_L1_menu(g_u8menuNO);		//第二行
		DISP_line3rd_menu_select();		//第三行
		
		/** voice prompt **/
		vp_stor(CVOPID_SET);
        promptDelay(TIMER_500MS);		//
		vp_stor(CVOPID_Press);
		vp_stor(CVOPID_asterisk);
		vp_stor(CVOPID_Hao);
    	vp_stor(CVOPID_RETURN);
		vp_stor(CVOPID_Press);
		vp_stor(CVOPID_pound);
		vp_stor(CVOPID_Hao);
    	vp_stor(CVOPID_CONFIRM);
		
		/** 只点亮2, 8, *, # **/
		led_off(0);
		led_on(2);
		led_on(8);
		led_on(10);
		led_on(12);
		break;

	case CMSG_BACK:
        g_u8menuNO = 0;
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        /** display **/
        OLED_cleanPages(0, 7);
		DISP_line1st_menu();			//第一行
		DISP_L1_menu(g_u8menuNO);		//第二行
		DISP_line3rd_menu_select();		//第三行
		
		/** voice prompt **/
		#if	0
		vp_stor(CVOPID_SET);
        promptDelay(TIMER_500MS);		//
		vp_stor(CVOPID_Press);
		vp_stor(CVOPID_asterisk);
		vp_stor(CVOPID_Hao);
    	vp_stor(CVOPID_RETURN);
		vp_stor(CVOPID_Press);
		vp_stor(CVOPID_pound);
		vp_stor(CVOPID_Hao);
    	vp_stor(CVOPID_CONFIRM);
		#endif
		
		/** 只点亮2, 8, *, # **/
		led_off(0);
		led_on(2);
		led_on(8);
		led_on(10);
		led_on(12);
		break;
		
	case CMSG_TMR:     
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
		}
		break;
				
	//case CMSG_BACK:
	case CFIGER_ON:		/** 检测到指纹按下 **/
		break;

	case CMSG_DKEY:							/** 检测到按键按下 **/	
		g_tick = 0;	
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		promptInit();
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CMAX, CMIN);
    		DISP_L1_menu(g_u8menuNO);
			break;
			
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		g_u8menuNO = value_inc(g_u8menuNO, CMAX, CMIN);
    		DISP_L1_menu(g_u8menuNO);
			break;
			
    	case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    	 
    		func.func = menuTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
			
    	case CKEY_SET:				/** 室内设置 **/
			ledPrompt(0, TIMER_100MS); 	//for test only
    		beep();
			break;
				
		default:					/** 可能的错误 **/
			break;
		}
		break;

	default:
		break;
	}  
#undef	CMIN
#undef	CMAX
    return  0;
}

/*******************************************************************************
 * 第一次登录，需要录入第一管理员
 * 录入第一管理者前的语音及显示提示
 *******************************************************************************/
int f_set_addSuperAdmin(unsigned *pMsg)
{	
    func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		actionInit();
		break;

	case CPMT_OVER:
		promptInit();
		
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
        func.func = f_set_addSuperAdminDoing;
    	fstack_push(&g_fstack, &func);
    	
		break;

	case CMSG_TMR:
		if(g_tick++ > 10) {
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
    	
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		break;
	
	case CMSG_INIT:	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		/** display **/
		DISP_L3_menu_line1st_addmanager();	//添加管理员
		DISP_menu_line2nd_PP(2);
		OLED_cleanPages(5, 7);		//第三行
		
		/** voice prompt **/
		vp_stor(CVOPID_ADD);
		vp_stor(CVOPID_MANAGER);
		vp_stor(CVOPID_Please);
		vp_stor(CVOPID_Press);
    	vp_stor(CVOPID_FINGER);
    	
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_SET:				/** 室内设置 **/
			break;
			
    	case CKEY_asterisk:
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		//beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
			break;
			
    	case CKEY_3:
    	case CKEY_5:
    	case CKEY_7:
    		promptInit();
    		beep();
    		
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);
			if(MisLOWPOWERCHECK("低电校验") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("恢复出厂设置") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
    		break;
    				
		default:					/** 可能的错误 **/
			break;
		}
		break;		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/***********************************************************
 * 录入第一管理指纹： 
 * 上电-->initial-->eroll-->success/fail
 ***********************************************************/
int f_set_addSuperAdminDoing(unsigned *pMsg)
{	
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms后初始化指纹传感器(JP100-A可以省略此步骤)
		//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_FGINIT);	//100ms后初始化指纹传感器
    	MFPOWER_ON();				//指纹传感器供电
    	
		g_u8pswordSel = 0;
        g_u8menuNO = 0;
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_ENROLL);
    		
			func.func = f_set_authenFail;
    		fstack_push(&g_fstack, &func);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
			
	case CFIGER_INIT:	//时使用JP100-A的场景下省略此步骤。)
		FP_INIT();
		break;
		
	//case CMSG_FGCCEL:				//确保指纹传感器初始化
	case CMSG_FGINIT:				//完成指纹传感器初始化
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	g_u8pswordNO = 0;
		//FP_Register(0, 0, g_u8pswordNO);	//BLH3001的指纹模板序号从0开始
		FP_Register(1, 1, g_u8pswordNO);	//JP100-A的指纹模板序号从1开始
		break;
		
	case CMSG_FGOPFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_ENROLL);
    	MFPOWER_OFF();				//指纹传感器断电
    	
		func.func = f_set_authenFail;
    	fstack_push(&g_fstack, &func);
		break;
			
	case CMSG_FGRGST:				//指纹录入完成
		MFPOWER_OFF();				//指纹传感器断电
		func.func = f_set_authenSucc;
    	fstack_push(&g_fstack, &func);
    		
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_ENROLL);
    		
    	/** byte图标志 **/
    	//fingerTab_setEntry(0, CENTRYFLAG_BUSY);
    	fingerTab_setEntry(1, CENTRYFLAG_BUSY);
		break;
		
	case CMSG_FGRGSTING:		//图像不清晰或正在持续处理
	case CMSG_FGDOING:			//命令数据错
	case CMSG_FGCOMFAIL:		//通信校验错。可以重试
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
		g_u8pswordNO++;	/** 新的JP100-A指纹传感器似乎用不上 **/		
		//FP_Register(0, 0, g_u8pswordNO);	//BLH3001的指纹模板序号从0开始
		FP_Register(1, 1, g_u8pswordNO);	//JP100-A的指纹模板序号从1开始
		
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		MFPOWER_OFF();
    		
    		ledPrompt(10, TIMER_100MS); 	//exit
    		promptInit();
    		//beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
    		func.func = f_idle;
    		fstack_push(&g_fstack, &func);
			break;
		
    	case CKEY_3:
    	case CKEY_5:
    	case CKEY_7:
    		promptInit();
    		beep();
    		
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);
			if(MisLOWPOWERCHECK("低电校验") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("恢复出厂设置") && SET_WAKECHECK("功能键")) {
				MFPOWER_OFF();					//关闭指纹传感器电源
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
    		break;
			
		default:				/** 可能的错误 **/
			break;
		}		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/***********************************************************
 * 成功进入菜单设置
 * 语音播放
 * 
 ***********************************************************/
int f_set_authenSucc(unsigned *pMsg)
{	
	func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CINIT_ENROLL:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    
		//DISP_line2nd_PFadmin_ID(M2AdminID(0));    
		DISP_line2nd_PFadmin_ID(M2AdminID(1));
		DISP_line3rd_AddSucc();
	
		vp_stor(CVOPID_ADD);
		vp_stor(CVOPID_SUCESS);
    	break;
    		
	case CINIT_MATCH:					/** 通过管理员认证 **/
		promptInit();
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
		
		OLED_cleanPages(0, 2);			//第三行    
		DISP_line2nd_PFadmin_ID(M2AdminID(MFPid_TOP(0)));
		DISP_PSverify_line3rd_succ(1);
    	break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		/** 建立功能栈 **/
    	fstack_init(&g_fstack);
		func.func = f_idle;
    	fstack_push(&g_fstack, &func);
		func.func = f_set_funcSel;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	//100ms后初始化指纹传感器
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/***********************************************************
 * 添加/验证第1管理员失败
 ***********************************************************/
int f_set_authenFail(unsigned *pMsg)
{	
	func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
		#if	1
	case CMSG_INIT:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
		OLED_cleanPages(2, 2);		//第三行
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//添加
		vp_stor(CVOPID_FAILED);		//失败
		break;
		#endif
		
	case CINIT_ENROLL:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
		OLED_cleanPages(2, 2);		//第三行
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//添加
		vp_stor(CVOPID_FAILED);		//失败
    	break;
    		
	case CINIT_MATCH:					/** 通过管理员认证 **/
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    
		DISP_PSverify_line3rd_succ(0);
		
		vp_stor(CVOPID_COMPARISON);	//验证
    	vp_stor(CVOPID_FAILED);		//失败
    	break;
    			
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_init(&g_fstack);
    	//func.func = f_mngFinger_addmngrDoing;
    	func.func = f_idle;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_init(&g_fstack);
    		func.func = f_idle;
    		fstack_push(&g_fstack, &func);
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}


/*******************************************************************************
 * 低电校准
 *******************************************************************************/
int f_set_lowPowerCali(unsigned *pMsg)
{
    func_t func;
    //msg_t	msg;
    unsigned short temp1;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	adcSample_Start(&g_adcData);	//启动采样定时
    	
    	OLED_cleanPages(0, 0);
    	DISP_lowPowerCheck();			//低电校准
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//请稍等...
		break;
					
	case CMSG_TMR:
		if(adcSample_Out(&g_adcData,  &temp1) == TRUE) {		/** ADC电压采样完成, 取均值 **/
			adcSample_Stop(&g_adcData);
			VOL_setRefValue(temp1);
			DISP_menu_MNGSYS_line3rd_succ(5);
			
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    		
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		
		if(g_tick++ >= 20) {	//超时
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		break;
	default:					/** 可能的错误 **/
		break;
	}	
	return	0;	
}

/*******************************************************************************
 * 语音开关
 *******************************************************************************/
int f_set_vop(unsigned *pMsg)
{
    func_t func;
    //msg_t	msg;
    //unsigned short temp1;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	adcSample_Start(&g_adcData);	//启动采样定时
    	
    	OLED_cleanPages(0, 0);
    	//DISP_lowPowerCheck();			//低电校准
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//请稍等...
    	beep();
    	vp_play(CVOPID_VOICE);					//????????????????
    	
		break;
			
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_init(&g_fstack);
    	func.func = f_idle;
    	fstack_push(&g_fstack, &func);
		break;
						
	case CMSG_TMR:
		if(g_tick++ >= 20) {	//超时
    		g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		break;
	default:					/** 可能的错误 **/
		break;
	}	
	return	0;	
}


/*******************************************************************************
 * 长按计时. 准备初始化整个系统
 *******************************************************************************/
int f_set_longPress(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		
    	OLED_cleanPages(0, 0);
    	//DISP_lowPowerCheck();			//低电校准
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//请稍等...
    	
		break;
			
	case CPMT_OVER:
		promptInit();
		break;
						
	case CMSG_TMR:
		if(g_tick++ >= 10) {	//超时
    		promptInit();
    		beep();
    		
			fstack_init(&g_fstack);
			func.func = f_mngRestoreFactory;
    		fstack_push(&g_fstack, &func);
    		
    		//g_u8pswordNO++;
    		//if(g_u8pswordNO == 1) {		//第n次按下#号
	    		OLED_cleanPages(1, 1);
				DISP_menu_MNGSYS_line2nd_RestoreDoing();	//恢复出厂
				//DISP_menu_MNGSYS_line3rd_RestoreDoing(6);
		    	OLED_cleanPages(5, 5);
				DISP_menu_MNGSYS_line3rd_Waiting(6);	//请稍等
    			MFPOWER_ON();				//指纹传感器供电
    			
				psword_delAll();	//全部密码
				//psword_delAll();	//全部蓝牙设备
				fingerTab_clean();	//指纹索引表
				
				record_clean();		//开锁记录
				
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_300MS, CFIGER_INIT);
    		//}
		}
		break;
		
	case CMSG_UKEY:			/** 检测到按键松开 **/
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:				/** 室内设置 **/
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_set;
    		fstack_push(&g_fstack, &func);
    		break;
		default:
			break;
    	}
		break;
	default:					/** 可能的错误 **/
		break;
	}	
	return	0;	
}
/////////////////////////////////////////////////////

