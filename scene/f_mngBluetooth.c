
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
#include "f_mngBluetooth.h"

u8 g_u8bluetooth[CBLUETOOTH_MAXLEN];	//最大长度蓝牙id的空间(接收用)

const static pfunc_t mngBluetoothTab[] = {
	f_mngBluetooth_add,
	f_mngBluetooth_del,
};
/*******************************************************************************
 * 密码管理
 * 选择添加/删除/虚位密码其中之一
 *******************************************************************************/
int f_mngBluetooth(unsigned *pMsg)
{
#define	CBLUETOOTH_MIN	0
#define	CBLUETOOTH_MAX	(MTABSIZE(mngBluetoothTab) - 1)

    func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	#if	1
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        g_u8menuNO = CBLUETOOTH_MIN;
        
        All_Screen(0);
		//DISP_menu_MNGPW_line1st();
		DISP_menu_MNGBT_line1st();		//第一行
		DISP_L2_menu_btmng(g_u8menuNO);	//第二行
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
    		g_tick = 0;
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CBLUETOOTH_MAX, CBLUETOOTH_MIN);
    		//DISP_L2_menu_pwmng(g_u8menuNO);	//第二行
    		DISP_L2_menu_btmng(g_u8menuNO);	//第二行
			break;
		
    	case CKEY_8:
    		g_tick = 0;
    		ledPrompt(11, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_inc(g_u8menuNO, CBLUETOOTH_MAX, CBLUETOOTH_MIN);
    		///DISP_L2_menu_pwmng(g_u8menuNO);	//第二行
    		DISP_L2_menu_btmng(g_u8menuNO);	//第二行
			break;
			
		case CKEY_asterisk:
    		g_tick = 0;
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
    		
    		func.func = mngBluetoothTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	#endif
	default:					/** 可能的错误 **/
		break;
	}
	return	0;
    
#undef	CBLUETOOTH_MIN
#undef	CBLUETOOTH_MAX
}

/*******************************************************************************
 * 添加蓝牙配对设备
 * 选择一个编号id(CBLUETOOTHID_MIN, CBLUETOOTHID_MAX)
 *******************************************************************************/
int f_mngBluetooth_add(unsigned *pMsg)
{
#define	CBLUETOOTHID_MIN	1
#define	CBLUETOOTHID_MAX	(CBLUETOOTHCNT - 1)
	
	#if	1
    func_t func;
    u8 i, j;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
        
        g_u8pswordSel = CBLUETOOTHID_MIN;
        for(i = 0; i < (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1); i++) {
        	if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        	} else {
        		break;
        	}
    	}
    	
    	if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)) {				//密码满
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_NEXT);	//准备进入下一阶段
    	}
        
		DISP_L3_menu_line1st_addBluetooth();	//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
		DISP_line3rd_menu_select();		//第三行
		break;	
	
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
		}		
		break;	
	
	case CMSG_NEXT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
		func.func = f_mngBluetooth_full;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
    		for(i = 0; i < (CBLUETOOTHID_MAX - CBLUETOOTHID_MIN + 1); i++) {
        		if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        		} else {
        			break;
        		}
    		}
    		
    		if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
    		for(i = 0; i < (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1); i++) {
        		if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        		} else {
        			break;
        		}
    		}
    		if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
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
    		
			func.func = f_mngBluetooth_AddDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** 可能的错误 **/
			break;
		}
	default:					/** 可能的错误 **/
		break;
	}
	#endif
	
	return	0;
    
#undef	CBLUETOOTHID_MIN
#undef	CBLUETOOTHID_MAX
}
/*******************************************************************************
 * 配对中，仅显示及发出声音提示, 等待收到的数据
 * 打开bluetooth设备接收数据，接收到或超时后关闭bluetooth设备
 *******************************************************************************/
int f_mngBluetooth_AddDoing(unsigned *pMsg)
{
	#if	1
    func_t func;	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		bluetooth_PWRon();	//power on
		
		//DISP_L3_menu_line2nd_inputPsword();
		OLED_cleanPages(2, 2);
		DISP_line3rd_addDoing(3);
		
		OLED_cleanPages(6, 7);
		g_u8pswordNO = 0;
		g_u8pswordLen = 0;
		
    	beep();
    	promptDelay(TIMER_1SEC);
    	beep();
    	promptDelay(TIMER_1SEC);
    	beep();
		break;	
	
	case CPMT_OVER:
		promptInit();
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick % 13), TIMER_100MS); 	//for test only        
		g_tick++;
		if(g_tick++ >= 60) {	/** 配对超时 **/
			func.func = f_mngBluetooth_AddFail;
    		fstack_push(&g_fstack, &func);
    		
    		//bluetooth_PWRoff();		//power off
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		break;	
		
	case CMSG_BTENROLL:
		promptInit();
    	beep();
    	//bluetooth_PWRoff();		//power off
    	
		func.func = f_mngBluetooth_AddSucc;
    	fstack_push(&g_fstack, &func);
    	
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		break;
				
	case CMSG_DKEY:								/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
			#if	0
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
			break;
			#endif
			
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);	
			break;
		
    	case CKEY_pound:
    		ledPrompt(10, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		#if	0
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		
    		g_u8pswordNO++;
			if(psword_inputConfirm(g_u8password) != 0) {
				func.func = f_mngBluetooth_AddSucc;
    			fstack_push(&g_fstack, &func);
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			} else {
				func.func = f_mngBluetooth_AddFail;
    			fstack_push(&g_fstack, &func);
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			}
			#endif
			break;
			
		default:					/** 可能的错误 **/
			break;
		}
			
	default:					/** 可能的错误 **/
		break;
	}
    #endif
    
	return	0;
}

int f_mngBluetooth_AddSucc(unsigned *pMsg)
{   
	#if	1
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
		DISP_line2nd_Bluetooth_ID(g_u8pswordSel);
        DISP_line3rd_AddSucc();
        
		vp_stor(CVOPID_ADD);
    	vp_stor(CVOPID_SUCESS);
    	
    	//psword_add(g_u8password, g_u8pswordSel);			//将g_u8password中的密码存入到g_u8pswordSel序号对应的地址
    	bluetoothEntry_add(g_u8bluetooth, g_u8pswordSel);	//将g_u8bluetooth中的蓝牙id存入到g_u8pswordSel序号对应的地址
    	
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
	#endif
	
	return  0;
}

int f_mngBluetooth_AddFail(unsigned *pMsg)
{
	#if	1
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
	#endif
	
	return  0;	
}

/*******************************************************************************
 * 删除配对设备
 * 选择一个编号id(CBLUETOOTHID_MIN, CBLUETOOTHID_MAX)
 *******************************************************************************/
int f_mngBluetooth_del(unsigned *pMsg)
{
#define	CBLUETOOTHID_MIN	1
#define	CBLUETOOTHID_MAX	(CBLUETOOTHCNT - 1)
	
    func_t func;
    u8 i, j;
    
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
        
        g_u8pswordSel = CBLUETOOTHID_MIN;
        for(i = 0; i < (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1); i++) {
        	if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        		break;
        	} else {
        		g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        	}
    	}
    	
    	if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)) {				//密码空
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_NEXT);	//准备进入下一阶段
    	}
        
		//DISP_L3_menu_line1st_delPsword();
		DISP_L3_menu_line1st_delBluetooth();	//第一行
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
		DISP_line3rd_menu_select();			//第三行
        nop();
		break;	

	case CPMT_OVER:
		promptInit();
		break;
			
	case CMSG_TMR:
		g_tick++;
		break;
		
	case CMSG_NEXT:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
		func.func = f_mngBluetooth_empty;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_DKEY:							/** 检测到按键按下 **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_dec(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
    		for(i = 0; i < (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1); i++) {
        		if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        			break;
        		} else {
        			g_u8pswordSel = value_dec(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        		}
    		}
    		if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)){		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_dec(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 
    		beep();
    		
    		j = g_u8pswordSel;
    		g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
    		for(i = 0; i < (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1); i++) {
        		if(bluetoothEntry_isBusy(g_u8pswordSel)) {
        			break;
        		} else {
        			g_u8pswordSel = value_inc(g_u8pswordSel, CBLUETOOTHID_MAX, CBLUETOOTHID_MIN);
        		}
    		}
    		if(i >= (CBLUETOOTHID_MAX  - CBLUETOOTHID_MIN + 1)) {		//如果没有找到可用索引，就用原来的
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CBLUETOOTHID_MAX);//第二行
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
    		
			func.func = f_mngBluetooth_delConfirm;
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
    
#undef	CBLUETOOTHID_MIN
#undef	CBLUETOOTHID_MAX
}

/*********************************************************************
 * 确认删除 ?
 *********************************************************************/
int f_mngBluetooth_delConfirm(unsigned *pMsg)
{
	#if	1
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
    		
			func.func = f_mngBluetooth_delDoing;
    		fstack_push(&g_fstack, &func);
			break;	
		default:				/** 可能的错误 **/
			break;
		}		
	default:					/** 可能的错误 **/
		break;
	}
	#endif
	
	return	0;
}
/*******************************************************************************
 * 正式删除
 *******************************************************************************/
int f_mngBluetooth_delDoing(unsigned *pMsg)
{
	#if	1
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		promptDelay(TIMER_100MS);
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms后初始化指纹头
    	
		//bluetoothEntry_del(g_u8pswordSel);
		bluetoothEntry_del(g_u8pswordSel);
		break;
		
	case CMSG_TMR:
		if(g_tick++ >= 2) {
			func.func = f_mngBluetooth_delSucc;
    		fstack_push(&g_fstack, &func);
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	}
    #endif
    
    return  0;
}

/*******************************************************************************
 * 操作成功语音提示
 *******************************************************************************/
int f_mngBluetooth_delSucc(unsigned *pMsg)
{   
	#if	1
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
	#endif
	
	return  0;
}

/*******************************************************************************
 * 密码空语音提示
 *******************************************************************************/
int f_mngBluetooth_empty(unsigned *pMsg)
{   
	#if	1
	//func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
        g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR); 
		
		OLED_cleanPages(2, 7);
        DISP_line3rd_empty(3);
        
		//vp_stor(CVOPID_PSWORD);
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
	#endif
	
	return  0;
}

/*******************************************************************************
 * 满语音提示
 *******************************************************************************/
int f_mngBluetooth_full(unsigned *pMsg)
{   
	#if	1
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
	#endif
	
	return  0;
}

/////////////////////////////////////////////////////
