
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "driver.h"

#include "l_fingprint.h"
#include "l_i2c_touch.h"
#include "l_oled.h"
#include "f_set.h"
#include "f_idle.h"
#include "f_mngfinger.h"

/***********************************************************
 * 全部变量用途
 * 指纹、密码功能的状态互斥
 * 其中：
 * g_u8menuNO - 作为功能号，指定四个功能中的一个
 * g_u8pswordSel - 作为ID号，指定要录入、删除的用户ID
 * g_u8pswordNO - 作为正在操作的步骤号，录入指纹时3步中的序号(JP100-A无需此)
 * g_u8pswordLen - 未用到
 ***********************************************************/

/***********************************************************
 * 功能栈深度
 * 录入指纹场景：f_idle(depth=1)-->f_set_funcSel(depth=2)-->f_mngFinger(depth=3)-->f_mngFinger_addmngr(depth=4)-->f_mngFinger_addmngrDoing(depth=5)-->f_mngFinger_addmngrSucc(6)
 * 删除指纹场景：f_idle(depth=1)-->f_set_funcSel(depth=2)-->f_mngFinger(depth=3)-->f_mngFinger_delmngr(depth=4)-->f_mngFinger_delmngrConfirm(depth=5)
 																												  f_mngFinger_delmngrDoing(depth=5)-->f_mngFinger_delmngrSucc(6)
 ***********************************************************/ 
/***********************************************************
 * 功能号到指纹编号的映射偏移关系
 * 其中对应关系与mngFingerTab有对应关系
 ***********************************************************/
const static u8 idxDeltaTab[] = {
	0, MUSER_ID_BASE, 0, MUSER_ID_BASE,
};

/** 功能表。注意其中要与显示顺序一致， 以免造成混乱 **/
const static pfunc_t mngFingerTab[] = {
	f_mngFinger_addmngr,
	f_mngFinger_adduser,
	f_mngFinger_delmngr,
	f_mngFinger_deluser,
};

/*******************************************************************************
 * 指纹管理
 * 
 * 选择以下功能之一 
 * 添加管理员、添加用户、删除管理员、删除用户
 *******************************************************************************/
int f_mngFinger(unsigned *pMsg)
{
#define	CFN_MIN	0
#define	CFN_MAX	3

    func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		break;

	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        All_Screen(0);
        g_u8menuNO = 0;
        
		DISP_menu_MNGFP_line1st();		//第一行
		DISP_L2_menu_fpmng(g_u8menuNO);	//第二行
		DISP_line3rd_menu_select();		//第三行
		break;	
		
	case CMSG_TMR:       
		if(++g_tick >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
		}
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CFN_MAX, CFN_MIN);
    		OLED_cleanPages(2, 4);
    		DISP_L2_menu_fpmng(g_u8menuNO);	//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_u8menuNO = value_inc(g_u8menuNO, CFN_MAX, CFN_MIN);
    		OLED_cleanPages(2, 4);
    		DISP_L2_menu_fpmng(g_u8menuNO);	//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		//beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_BACK);
    		//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		func.func = mngFingerTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
    
#undef	CFN_MIN
#undef	CFN_MAX
}

/*******************************************************************************
 * 指定要添加的管理员ID
 * 
 *******************************************************************************/
int f_mngFinger_addmngr(unsigned *pMsg)
{
#define	CFADMIN_MIN	2
#define	CFADMIN_MAX	5
	func_t func;
    u8 i, j;
    u16	u16Temp;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        All_Screen(0);
		
        g_u8pswordSel = CFADMIN_MIN;       
        for(i = 0; i < (CFADMIN_MAX  - CFADMIN_MIN + 1); i++) {
       		//if(fingerTab_entryIsBusy(g_u8pswordSel)) {
       		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       		if(fingerTab_entryIsBusy(u16Temp)) {
       			g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
       		} else {
       			break;
       		}
    	}
    	
    	if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)) {				//指纹满
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//准备进入下一阶段
    		
    		func.func = f_mngFinger_full;
    		fstack_push(&g_fstack, &func);
    	}
        
		DISP_L3_menu_line1st_addmanager();	//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
		DISP_line3rd_menu_select();		//第三行
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
		}
		
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		for(i = 0; i < (CFADMIN_MAX - CFADMIN_MIN + 1); i++) {
        		if(fingerTab_entryIsBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		for(i = 0; i < (CFADMIN_MAX  - CFADMIN_MIN + 1); i++) {
        		if(fingerTab_entryIsBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		//g_u8pswordSel = g_u8pswordSel;		//管理员ID号
			func.func = f_mngFinger_addmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
	
#undef	CFADMIN_MIN
#undef	CFADMIN_MAX
}

/**
 * 录入指纹全部： 
 * 上电-->initial-->eroll-->success/fail
 **/
int f_mngFinger_addmngrDoing(unsigned *pMsg)
{	
	func_t func;
	u16 u16Temp;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    	MFPOWER_ON();				//指纹传感器供电
    	
		//DISP_L3_menu_line1st_manager();	//第一行
		DISP_menu_line2nd_PP(2);
		OLED_cleanPages(5, 7);		//第三行
		
		vp_stor(CVOPID_Please);
		vp_stor(CVOPID_Press);
    	vp_stor(CVOPID_FINGER);
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms后初始化指纹传感器
		break;
			
	case CFIGER_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		FP_INIT();
		break;
		
	case CMSG_FGINIT:				//完成指纹传感器初始化
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	//g_u8pswordNO = 0;
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_Register(u16Temp, u16Temp, 0);
		break;
		
	case CMSG_FGOPFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//指纹传感器断电
    	
		func.func = f_mngFinger_addmngrFail;
    	fstack_push(&g_fstack, &func);
		break;
	
	case CMSG_FGRGSTING:		//图像不清晰或正在持续处理
	case CMSG_FGDOING:			//命令数据错
	case CMSG_FGCOMFAIL:		//通信校验错。可以重试
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
		//g_u8pswordNO++;
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_Register(u16Temp, u16Temp, 0);	
		break;
					
	case CMSG_FGRGST:				//完成指纹传感器初始化
    	MFPOWER_OFF();				//指纹传感器断电
		func.func = f_mngFinger_addmngrSucc;
    	fstack_push(&g_fstack, &func);
    	
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	/** FLASH标志操作 **/
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
    	fingerTab_setEntry(u16Temp, CENTRYFLAG_BUSY);
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		MFPOWER_OFF();
    		
    		ledPrompt(10, TIMER_100MS); 	//exit
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
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
 * 录入成功语音播放
 *
 ***********************************************************/
int f_mngFinger_addmngrSucc(unsigned *pMsg)
{	
	//func_t func;
    u16 u16Temp;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    	
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		OLED_cleanPages(2, 7);		//第三行(清空)
		if(MisADMIN(u16Temp)) {
			//DISP_line2nd_PFadmin_ID(2);
        	DISP_line2nd_PFadmin_ID(M2AdminID(u16Temp));
		} else {
			//DISP_line2nd_PFuser_ID(2);
			DISP_line2nd_PFuser_ID(M2UserID(u16Temp));
		}
		DISP_line3rd_AddSucc();
		
		vp_stor(CVOPID_ADD);
		vp_stor(CVOPID_SUCESS);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//两步回到选择ID的阶段
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//两步回到选择ID的阶段
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

int f_mngFinger_addmngrFail(unsigned *pMsg)
{	//func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    	
		//DISP_L3_menu_line1st_manager();	//第一行
		OLED_cleanPages(2, 2);		//第三行
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//添加
		vp_stor(CVOPID_FAILED);		//失败
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//两步回到选择ID的阶段
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//两步回到选择ID的阶段
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}
/*******************************************************************************
 * 指定要添加的用户ID
 * 
 *******************************************************************************/
int f_mngFinger_adduser(unsigned *pMsg)
{
#define	CFUSER_MIN	1
#define	CFUSER_MAX	200

	func_t func;
    u8 i, j;
    u16 u16Temp;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        All_Screen(0);
        
        g_u8pswordSel = CFUSER_MIN;   
        for(i = 0; i < (CFUSER_MAX  - CFUSER_MIN + 1); i++) {
       		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       		if(fingerTab_entryIsBusy(u16Temp)) {
       			g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
       		} else {
       			break;
       		}
    	}
    	if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)) {				//指纹满
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//准备进入下一阶段
    		
    		func.func = f_mngFinger_full;
    		fstack_push(&g_fstack, &func);
    	} else {
			DISP_L3_menu_line1st_adduser();	//第一行
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			DISP_line3rd_menu_select();		//第三行
		}
		break;
		
	case CMSG_TMR:
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
    		for(i = 0; i < (CFUSER_MAX - CFUSER_MIN + 1); i++) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
    		for(i = 0; i < (CFUSER_MAX  - CFUSER_MIN + 1); i++) {
        		//if(fingerTab_entryIsBusy(g_u8pswordSel + )) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		//g_u8pswordSel = g_u8pswordSel;		//管理员ID号
			func.func = f_mngFinger_addmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
	
#undef	CFUSER_MIN
#undef	CFUSER_MAX
}

/*******************************************************************************
 * 选择一个要删除的管理员ID号
 *******************************************************************************/
int f_mngFinger_delmngr(unsigned *pMsg)
{
#define	CFADMIN_MIN	2
#define	CFADMIN_MAX	5

	func_t func;
    u16 u16Temp;
    u8 i, j;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        All_Screen(0);
        
        g_u8pswordSel = CFADMIN_MIN;
        for(i = 0; i < (CFADMIN_MAX  - CFADMIN_MIN + 1); i++) {
        	//if(fingerTab_entryIsBusy(g_u8pswordSel)) {
       		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       		if(fingerTab_entryIsBusy(u16Temp)) {
        		break;
        	} else {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
        	}
    	}
    	if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)) {				//密码空
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//准备进入下一阶段
    		
			func.func = f_mngFinger_empty;
    		fstack_push(&g_fstack, &func);
    	}
        
		DISP_L3_menu_line1st_delmanager();	//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
		DISP_line3rd_menu_select();		//第三行
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		for(i = 0; i < (CFADMIN_MAX  - CFADMIN_MIN + 1); i++) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			break;
        		} else {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
        		}
    		}
    		if(i >= (CFADMIN_MAX  - CFADMIN_MAX + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		for(i = 0; i < (CFADMIN_MAX  - CFADMIN_MIN + 1); i++) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			break;
        		} else {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
        		}
    		}
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		//g_u8pswordSel = g_u8pswordSel;		//管理员ID号
			func.func = f_mngFinger_delmngrConfirm;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
	
#undef	CFADMIN_MIN
#undef	CFADMIN_MAX
}

/*********************************************************************
 * 确认删除 ?
 *********************************************************************/
int f_mngFinger_delmngrConfirm(unsigned *pMsg)
{
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//计时
    	
		DISP_L3_menu_line2nd_confirmDelFingprint();	//确认删除？
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_menu_confirm(6);				//否*  是#
		
		vp_stor(CVOPID_CONFIRM);	//确认
    	vp_stor(CVOPID_DEL);		//删除
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;

	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//exit
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
					
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);	//去除确认级功能. 以ID号选择作为上级功能
			func.func = f_mngFinger_delmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;	
		default:				/** 可能的错误 **/
			break;
		}		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * 
 *******************************************************************************/
int f_mngFinger_delmngrDoing(unsigned *pMsg)
{
	func_t func;
	u16 u16Temp;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms后初始化指纹传感器
    	MFPOWER_ON();				//指纹传感器供电
    	g_u8pswordNO = 0;
    	
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_delDoing(3);	//删除中...
		
		//vp_stor(CVOPID_Doing);	//正在
    	//vp_stor(CVOPID_DEL);		//删除
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	case CFIGER_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CFIGER_INIT); //!!!!如果没有被
		
		g_u8pswordNO++;		//try counter
		if(g_u8pswordNO > 3) {
			goto GOTO_DELETEFAIL;
		} else {
			FP_INIT();
		}
		break;
		
	case CMSG_FGINIT:				//完成指纹传感器初始化. 可以进行删除了
    	g_u8pswordNO = 0;	
    	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_DeRegister(u16Temp, u16Temp);
		break;
		
	case CMSG_FGCOMFAIL:		//通信故障，再来一次
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_DeRegister(u16Temp, u16Temp);
		break;
		
	case CMSG_FGOPFAIL:
	GOTO_DELETEFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//指纹传感器断电
    	
		func.func = f_mngFinger_delmngrFail;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_FGDERG:				//完成指纹传感器初始化
		promptInit();
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//指纹传感器断电
    	
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
    	fingerTab_setEntry(u16Temp, CENTRYFLAG_IDLE);
    	
		func.func = f_mngFinger_delmngrSucc;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/***********************************************************
 * 
 ***********************************************************/
int f_mngFinger_delmngrSucc(unsigned *pMsg)
{	
	//func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    	
		OLED_cleanPages(0, 7);		//(清空)
		DISP_line3rd_delSucc(3);	//删除成功
		
		vp_stor(CVOPID_DEL);
		vp_stor(CVOPID_SUCESS);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//2步回到选择ID的阶段(ID选择-->动作执行-->此处)
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//2步回到选择ID的阶段(ID选择-->动作执行-->此处)
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

int f_mngFinger_delmngrFail(unsigned *pMsg)
{
	//func_t func;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹传感器
    	
		OLED_cleanPages(0, 7);		//(清空)
		DISP_line3rd_delFail(3);	//删除失败
		
		vp_stor(CVOPID_DEL);
		vp_stor(CVOPID_FAILED);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//两步回到选择ID的阶段
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//两步回到选择ID的阶段
    	}
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * 选择一个要删除的用户ID号
 *******************************************************************************/
int f_mngFinger_deluser(unsigned *pMsg)
{
#define	CFUSER_MIN	1
#define	CFUSER_MAX	200

	func_t func;
    u16 u16Temp;
    u8 i, j;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        All_Screen(0);
        
        g_u8pswordSel = CFUSER_MIN;
        for(i = 0; i < (CFUSER_MAX  - CFUSER_MIN + 1); i++) {
       		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       		if(fingerTab_entryIsBusy(u16Temp)) {
        		break;
        	} else {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
        	}
    	}
    	if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)) {				//指纹空
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//准备进入下一阶段
    		
			func.func = f_mngFinger_empty;
    		fstack_push(&g_fstack, &func);
    	} else {
			DISP_L3_menu_line1st_deluser();	//第一行
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			DISP_line3rd_menu_select();		//第三行
		}
		break;
	
	case CMSG_NEXT:
		
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
    		for(i = 0; i < (CFUSER_MAX - CFUSER_MIN + 1); i++) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			break;
        		} else {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
        		}
    		}
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
    		for(i = 0; i < (CFUSER_MAX  - CFUSER_MIN + 1); i++) {
       			u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
       			if(fingerTab_entryIsBusy(u16Temp)) {
        			break;
        		} else {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CFUSER_MAX, CFUSER_MIN);
        		}
    		}
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}

			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		//g_u8pswordSel = g_u8pswordSel;		//用户ID号
			func.func = f_mngFinger_delmngrConfirm;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
	
#undef	CFUSER_MIN
#undef	CFUSER_MAX
}

/*******************************************************************************
 * 指纹空语音提示
 *******************************************************************************/
int f_mngFinger_empty(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
        DISP_line3rd_empty(3);
        
		vp_stor(CVOPID_Had);
    	vp_stor(CVOPID_CLEARED);
		break;	
	
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS);
		g_tick++;
		if(g_tick > 3) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//退2步
		}
		break;	
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//退2步
		break;
			
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;
}

/*******************************************************************************
 * 指纹满语音提示
 *******************************************************************************/
int f_mngFinger_full(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
        DISP_line3rd_empty(3);
        
		vp_stor(CVOPID_Had);
    	vp_stor(CVOPID_Full);
		break;	
	
	case CMSG_TMR:
		g_tick++;
		if(g_tick > 3) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//退2步
		}
		break;	
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//退2步
		break;
			
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;
}

/////////////////////////////////////////////////////

