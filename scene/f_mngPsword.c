
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
#include "f_mngPsword.h"

const static pfunc_t mngPswordTab[] = {
	f_mngPsword_add,
	f_mngPsword_del,
	f_mngPsword_Img,
};
/*******************************************************************************
 * 密码管理
 * 选择添加/删除/虚位密码其中之一
 *******************************************************************************/
int f_mngPsword(unsigned *pMsg)
{
#define	CPSWORDOP_MIN	0
#define	CPSWORDOP_MAX	(MTABSIZE(mngPswordTab) - 1)

    func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        g_u8menuNO = CPSWORDOP_MIN;
        
        All_Screen(0);
		DISP_menu_MNGPW_line1st();		//第一行
		DISP_L2_menu_pwmng(g_u8menuNO);	//第二行
		OLED_cleanPages(5, 5);
		DISP_line3rd_menu_select();		//第三行
		break;	
	
	case CMSG_TMR:
		if(++g_tick >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_BACK);
    		fstack_pop(&g_fstack);
		}
		break;
	
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CPSWORDOP_MAX, CPSWORDOP_MIN);
    		DISP_L2_menu_pwmng(g_u8menuNO);	//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(11, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_inc(g_u8menuNO, CPSWORDOP_MAX, CPSWORDOP_MIN);
    		DISP_L2_menu_pwmng(g_u8menuNO);	//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_BACK);
    		//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    		
    		func.func = mngPswordTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
    
#undef	CPSWORDOP_MIN
#undef	CPSWORDOP_MAX
}

/*******************************************************************************
 * 添加密码
 * 选择一个编号id(CPSWORDID_MIN, CPSWORDID_MAX)
 *******************************************************************************/
int f_mngPsword_add(unsigned *pMsg)
{
#define	CPSWORDID_MIN	1
#define	CPSWORDID_MAX	(CPSWORDCNT - 1)
	
    func_t func;
    u8 i, j;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
        
        g_u8pswordSel = CPSWORDID_MIN;
        for(i = 0; i < (CPSWORDID_MAX  - CPSWORDID_MIN + 1); i++) {
        	if(psword_isBusy(g_u8pswordSel)) {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        	} else {
        		break;
        	}
    	}
    	
    	if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)) {				//密码满
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_NEXT);	//准备进入下一阶段
    	}
        
		DISP_L3_menu_line1st_addPsword();		//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
		DISP_line3rd_menu_select();		//第三行
		break;	
	
	case CMSG_TMR:
		g_tick++;
		break;	
	
	case CMSG_NEXT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
		func.func = f_mngPsword_full;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
    		for(i = 0; i < (CPSWORDID_MAX - CPSWORDID_MIN + 1); i++) {
        		if(psword_isBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        		} else {
        			break;
        		}
    		}
    		
    		if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
    		for(i = 0; i < (CPSWORDID_MAX  - CPSWORDID_MIN + 1); i++) {
        		if(psword_isBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		//g_u8pswordSel = g_u8pswordSel;
			func.func = f_mngPsword_AddDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
    
#undef	CPSWORDID_MIN
#undef	CPSWORDID_MAX
}
/*******************************************************************************
 * 输入中
 *******************************************************************************/
int f_mngPsword_AddDoing(unsigned *pMsg)
{
    func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		DISP_L3_menu_line2nd_inputPsword();
		
		OLED_cleanPages(6, 7);
		g_u8pswordNO = 0;
		g_u8pswordLen = 0;
		MPSWORD_CLEAN(x);
        DISP_line3rd_psword(MPSWORD_GETLEN(0)); 
        
		vp_stor(CVOPID_Please);
		vp_stor(CVOPID_INPUT);
    	vp_stor(CVOPID_PSWORD);
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
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
    		beep();
    		ledPrompt(((msg_t *)pMsg)->msgValue, TIMER_100MS);
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);
			DISP_line3rd_psword(MPSWORD_GETLEN(x) - g_u8pswordLen);
			
			//DISP_line3rd_array(g_u8password, g_u8password[0] + 1);	//!!!!!!!for debug
			break;
			
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		beep();
    		
    		if(MPSWORD_GETLEN(x) == 0) {
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			fstack_pop(&g_fstack);
    		} else {
				MPSWORD_POP("back");
				DISP_line3rd_psword(MPSWORD_GETLEN(0));
				
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			}
			break;
			
    	case CKEY_pound:
    		ledPrompt(10, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		
    		g_u8pswordNO++;
			if(g_u8pswordNO == 1) {	/** 第一次输入# **/
				if(MPSWORD_GETLEN("get password length") < CPSWORD_MINLEN) {
					func.func = f_mngPsword_AddFail;
    				fstack_push(&g_fstack, &func);
    				
    				SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
				} else {
					/** prompt "input psword again" **/
					g_u8pswordLen = MPSWORD_GETLEN(0);
					DISP_line3rd_psword(MPSWORD_GETLEN(x) - g_u8pswordLen);
					
					vp_stor(CVOPID_Please);
					vp_stor(CVOPID_AGAIN);
					vp_stor(CVOPID_INPUT);
    				vp_stor(CVOPID_PSWORD);
				}
			} else if (g_u8pswordNO == 2) {	/** 第二次输入#, 比对两次是否一致 **/
				if(psword_inputConfirm(g_u8password) != 0) {
					func.func = f_mngPsword_AddSucc;
    				fstack_push(&g_fstack, &func);
    				SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
				} else {
					func.func = f_mngPsword_AddFail;
    				fstack_push(&g_fstack, &func);
    				SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
				}
			} else {
				/** error **/
			}
			break;
		default:					/** 可能的错误 **/
			break;
		}
			
	default:					/** 可能的错误 **/
		break;
	}
    
	return	0;
}

int f_mngPsword_AddSucc(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
		DISP_line2nd_PSword_ID(g_u8pswordSel);
        DISP_line3rd_AddSucc();
        
		vp_stor(CVOPID_ADD);
    	vp_stor(CVOPID_SUCESS);
    	
    	psword_add(g_u8password, g_u8pswordSel);	//将g_u8password中的密码存入到g_u8pswordSel序号对应的地址
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
		g_tick++;
		if(g_tick > 1) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//退两步
		}
		break;	
		
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;
}
int f_mngPsword_AddFail(unsigned *pMsg)
{
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(0, 7);
        DISP_line3rd_AddFail();
        
		vp_stor(CVOPID_ADD);
    	vp_stor(CVOPID_FAILED);
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
		g_tick++;
		if(g_tick > 1) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//退两步
		}
		break;	
		
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;	
}
/*******************************************************************************
 * 删除一个ID号
 * 选择一个编号id(CPSWORDID_MIN, CPSWORDID_MAX)
 *******************************************************************************/
int f_mngPsword_del(unsigned *pMsg)
{
#define	CPSWORDID_MIN	1
#define	CPSWORDID_MAX	(CPSWORDCNT - 1)
	
    func_t func;
    u8 i, j;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
        
        g_u8pswordSel = CPSWORDID_MIN;
        for(i = 0; i < (CPSWORDID_MAX  - CPSWORDID_MIN + 1); i++) {
        	if(psword_isBusy(g_u8pswordSel)) {
        		break;
        	} else {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        	}
    	}
    	
    	if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)) {				//密码空
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_NEXT);	//准备进入下一阶段
    	}
        
		DISP_L3_menu_line1st_delPsword();	//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
		DISP_line3rd_menu_select();			//第三行
		break;	
	
	case CMSG_TMR:
		g_tick++;
		break;
		
	case CMSG_NEXT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
		func.func = f_mngPsword_empty;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
    		for(i = 0; i < (CPSWORDID_MAX  - CPSWORDID_MIN + 1); i++) {
        		if(psword_isBusy(g_u8pswordSel)) {
        			break;
        		} else {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        		}
    		}
    		if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_dec(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
    		for(i = 0; i < (CPSWORDID_MAX  - CPSWORDID_MIN + 1); i++) {
        		if(psword_isBusy(g_u8pswordSel)) {
        			break;
        		} else {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CPSWORDID_MAX, CPSWORDID_MIN);
        		}
    		}
    		if(i >= (CPSWORDID_MAX  - CPSWORDID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CPSWORDID_MAX);//第二行
			break;
				
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS); 
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
			func.func = f_mngPsword_delConfirm;
    		fstack_push(&g_fstack, &func);
    		
			DISP_L3_menu_line2nd_confirmDelPsword();
			OLED_cleanPages(6, 7);
			DISP_line3rd_menu_confirm(6);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
    
#undef	CPSWORDID_MIN
#undef	CPSWORDID_MAX
}

/*********************************************************************
 * 确认删除 ?
 *********************************************************************/
int f_mngPsword_delConfirm(unsigned *pMsg)
{
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms后初始化指纹头
    	
		DISP_L3_menu_line2nd_confirmDelPsword();	//确认删除？
		OLED_cleanPages(5, 7);		//第三行
		DISP_line3rd_menu_confirm(6);
		
		vp_stor(CVOPID_CONFIRM);	//确认
    	vp_stor(CVOPID_DEL);		//删除
		break;
		
	case CMSG_TMR:
		g_tick++;
		break;
	
	case CPMT_OVER:
		promptInit();
		break;

	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//exit
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
					
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		beep();
    		
    		OLED_cleanPages(2, 7);		//第三行
			DISP_line3rd_delDoing(3);	//删除中...
		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
			func.func = f_mngPsword_delDoing;
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
 * 正式删除
 *******************************************************************************/
int f_mngPsword_delDoing(unsigned *pMsg)
{
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		promptDelay(TIMER_100MS);
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹头
    	
		psword_del(g_u8pswordSel);
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 2) {
			func.func = f_mngPsword_delSucc;
    		fstack_push(&g_fstack, &func);
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	}
    
    return  0;
}

/*******************************************************************************
 * 操作成功语音提示
 *******************************************************************************/
int f_mngPsword_delSucc(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
		//DISP_line2nd_PSword_ID(g_u8pswordSel);
        DISP_line3rd_delSucc(3);
        
		vp_stor(CVOPID_DEL);
    	vp_stor(CVOPID_SUCESS);
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
		g_tick++;
		if(g_tick > 3) {
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//退3步
		}
		break;	
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//退3步
		break;
		
	default:					/** 可能的错误 **/
		break;	
	}
	return  0;
}

/*******************************************************************************
 * 密码空语音提示
 *******************************************************************************/
int f_mngPsword_empty(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
        DISP_line3rd_empty(3);
        
		vp_stor(CVOPID_PSWORD);
		vp_stor(CVOPID_Had);
    	vp_stor(CVOPID_CLEARED);
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
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

/*****************************************************************************
 * 满语音提示
 *******************************************************************************/
int f_mngPsword_full(unsigned *pMsg)
{   
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
        DISP_line3rd_full(3);
        
		vp_stor(CVOPID_PSWORD);
		vp_stor(CVOPID_Had);
    	vp_stor(CVOPID_Full);
		break;	
	
	case CMSG_TMR:
        ledPrompt((g_tick%13), TIMER_100MS);
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
 * 虚位密码
 *******************************************************************************/
int f_mngPsword_Img(unsigned *pMsg)
{
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
		OLED_cleanPages(0, 1);							//第1行
		//DISP_menu_MNGSYS_line2nd_EmptyPswdConfirm();	//启用虚位密码？
		//DISP_line3rd_menu_confirm(6);
        DISP_L3_menu_line2nd_ImgPsword();
		DISP_line3rd_menu_enable(6);
		//g_u8pswordNO = 0;
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
            MdisableImgPswd("禁用虚位密码功能");
            goto	GOTODOING;
    		
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
            MenableImgPswd("启用虚位密码功能");
    		
    	GOTODOING:
    		promptInit();
    		beep();
			DISP_menu_MNGSYS_line3rd_Waiting(6);	//请稍等
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_200MS, CMSG_INIT);
    	
			func.func = f_mngPsword_ImgDone;
    		fstack_push(&g_fstack, &func);
			break;
			
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * 设置完成
 *******************************************************************************/
int f_mngPsword_ImgDone(unsigned *pMsg)
{
	//func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
		//DISP_menu_MNGSYS_line3rd_over(5);
		DISP_line3rd_menu_enableStat(isMmgPswdEnable());
		promptDelay(TIMER_500MS);
		
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		break;
	case CMSG_TMR:
		if(g_tick++ >= 2) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
}
/////////////////////////////////////////////////////

