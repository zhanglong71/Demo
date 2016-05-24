
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
 * ȫ��������;
 * ָ�ơ����빦�ܵ�״̬����
 * ���У�
 * g_u8menuNO - ��Ϊ���ܺţ�ָ���ĸ������е�һ��
 * g_u8pswordSel - ��ΪID�ţ�ָ��Ҫ¼�롢ɾ�����û�ID
 * g_u8pswordNO - ��Ϊ���ڲ����Ĳ���ţ�¼��ָ��ʱ3���е����(JP100-A�����)
 * g_u8pswordLen - δ�õ�
 ***********************************************************/

/***********************************************************
 * ����ջ���
 * ¼��ָ�Ƴ�����f_idle(depth=1)-->f_set_funcSel(depth=2)-->f_mngFinger(depth=3)-->f_mngFinger_addmngr(depth=4)-->f_mngFinger_addmngrDoing(depth=5)-->f_mngFinger_addmngrSucc(6)
 * ɾ��ָ�Ƴ�����f_idle(depth=1)-->f_set_funcSel(depth=2)-->f_mngFinger(depth=3)-->f_mngFinger_delmngr(depth=4)-->f_mngFinger_delmngrConfirm(depth=5)
 																												  f_mngFinger_delmngrDoing(depth=5)-->f_mngFinger_delmngrSucc(6)
 ***********************************************************/ 
/***********************************************************
 * ���ܺŵ�ָ�Ʊ�ŵ�ӳ��ƫ�ƹ�ϵ
 * ���ж�Ӧ��ϵ��mngFingerTab�ж�Ӧ��ϵ
 ***********************************************************/
const static u8 idxDeltaTab[] = {
	0, MUSER_ID_BASE, 0, MUSER_ID_BASE,
};

/** ���ܱ�ע������Ҫ����ʾ˳��һ�£� ������ɻ��� **/
const static pfunc_t mngFingerTab[] = {
	f_mngFinger_addmngr,
	f_mngFinger_adduser,
	f_mngFinger_delmngr,
	f_mngFinger_deluser,
};

/*******************************************************************************
 * ָ�ƹ���
 * 
 * ѡ�����¹���֮һ 
 * ��ӹ���Ա������û���ɾ������Ա��ɾ���û�
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
        
		DISP_menu_MNGFP_line1st();		//��һ��
		DISP_L2_menu_fpmng(g_u8menuNO);	//�ڶ���
		DISP_line3rd_menu_select();		//������
		break;	
		
	case CMSG_TMR:       
		if(++g_tick >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
		}
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CFN_MAX, CFN_MIN);
    		OLED_cleanPages(2, 4);
    		DISP_L2_menu_fpmng(g_u8menuNO);	//�ڶ���
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
    		g_u8menuNO = value_inc(g_u8menuNO, CFN_MAX, CFN_MIN);
    		OLED_cleanPages(2, 4);
    		DISP_L2_menu_fpmng(g_u8menuNO);	//�ڶ���
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
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
    
#undef	CFN_MIN
#undef	CFN_MAX
}

/*******************************************************************************
 * ָ��Ҫ��ӵĹ���ԱID
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
    	
    	if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)) {				//ָ����
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//׼��������һ�׶�
    		
    		func.func = f_mngFinger_full;
    		fstack_push(&g_fstack, &func);
    	}
        
		DISP_L3_menu_line1st_addmanager();	//��һ��
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
		DISP_line3rd_menu_select();		//������
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_pop(&g_fstack);
		}
		
		break;	
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
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
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
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
    		
    		//g_u8pswordSel = g_u8pswordSel;		//����ԱID��
			func.func = f_mngFinger_addmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CFADMIN_MIN
#undef	CFADMIN_MAX
}

/**
 * ¼��ָ��ȫ���� 
 * �ϵ�-->initial-->eroll-->success/fail
 **/
int f_mngFinger_addmngrDoing(unsigned *pMsg)
{	
	func_t func;
	u16 u16Temp;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    	MFPOWER_ON();				//ָ�ƴ���������
    	
		//DISP_L3_menu_line1st_manager();	//��һ��
		DISP_menu_line2nd_PP(2);
		OLED_cleanPages(5, 7);		//������
		
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
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms���ʼ��ָ�ƴ�����
		break;
			
	case CFIGER_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		FP_INIT();
		break;
		
	case CMSG_FGINIT:				//���ָ�ƴ�������ʼ��
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	//g_u8pswordNO = 0;
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_Register(u16Temp, u16Temp, 0);
		break;
		
	case CMSG_FGOPFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
    	
		func.func = f_mngFinger_addmngrFail;
    	fstack_push(&g_fstack, &func);
		break;
	
	case CMSG_FGRGSTING:		//ͼ�����������ڳ�������
	case CMSG_FGDOING:			//�������ݴ�
	case CMSG_FGCOMFAIL:		//ͨ��У�����������
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
		//g_u8pswordNO++;
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_Register(u16Temp, u16Temp, 0);	
		break;
					
	case CMSG_FGRGST:				//���ָ�ƴ�������ʼ��
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
		func.func = f_mngFinger_addmngrSucc;
    	fstack_push(&g_fstack, &func);
    	
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	/** FLASH��־���� **/
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
    	fingerTab_setEntry(u16Temp, CENTRYFLAG_BUSY);
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
			
		default:				/** ���ܵĴ��� **/
			break;
		}		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/***********************************************************
 * ¼��ɹ���������
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    	
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		OLED_cleanPages(2, 7);		//������(���)
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
    	fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
    	}
		break;
		
	default:					/** ���ܵĴ��� **/
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    	
		//DISP_L3_menu_line1st_manager();	//��һ��
		OLED_cleanPages(2, 2);		//������
		OLED_cleanPages(5, 7);		//������
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//���
		vp_stor(CVOPID_FAILED);		//ʧ��
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
    	}
		break;
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}
/*******************************************************************************
 * ָ��Ҫ��ӵ��û�ID
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
    	if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)) {				//ָ����
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//׼��������һ�׶�
    		
    		func.func = f_mngFinger_full;
    		fstack_push(&g_fstack, &func);
    	} else {
			DISP_L3_menu_line1st_adduser();	//��һ��
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
			DISP_line3rd_menu_select();		//������
		}
		break;
		
	case CMSG_TMR:
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
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
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
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
    		
    		//g_u8pswordSel = g_u8pswordSel;		//����ԱID��
			func.func = f_mngFinger_addmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CFUSER_MIN
#undef	CFUSER_MAX
}

/*******************************************************************************
 * ѡ��һ��Ҫɾ���Ĺ���ԱID��
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
    	if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)) {				//�����
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//׼��������һ�׶�
    		
			func.func = f_mngFinger_empty;
    		fstack_push(&g_fstack, &func);
    	}
        
		DISP_L3_menu_line1st_delmanager();	//��һ��
		DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
		DISP_line3rd_menu_select();		//������
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
    		if(i >= (CFADMIN_MAX  - CFADMIN_MAX + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
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
    		if(i >= (CFADMIN_MAX  - CFADMIN_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
    		//g_u8pswordSel = value_inc(g_u8pswordSel, CFADMIN_MAX, CFADMIN_MIN);
    		
			DISP_line2nd_menu_IDxxSel(g_u8pswordSel, CFADMIN_MAX);//�ڶ���
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
    		
    		//g_u8pswordSel = g_u8pswordSel;		//����ԱID��
			func.func = f_mngFinger_delmngrConfirm;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CFADMIN_MIN
#undef	CFADMIN_MAX
}

/*********************************************************************
 * ȷ��ɾ�� ?
 *********************************************************************/
int f_mngFinger_delmngrConfirm(unsigned *pMsg)
{
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//��ʱ
    	
		DISP_L3_menu_line2nd_confirmDelFingprint();	//ȷ��ɾ����
		OLED_cleanPages(5, 7);		//������
		DISP_line3rd_menu_confirm(6);				//��*  ��#
		
		vp_stor(CVOPID_CONFIRM);	//ȷ��
    	vp_stor(CVOPID_DEL);		//ɾ��
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

	case CMSG_DKEY:							/** ��⵽�������� **/		
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
    		
    		fstack_pop(&g_fstack);	//ȥ��ȷ�ϼ�����. ��ID��ѡ����Ϊ�ϼ�����
			func.func = f_mngFinger_delmngrDoing;
    		fstack_push(&g_fstack, &func);
			break;	
		default:				/** ���ܵĴ��� **/
			break;
		}		
	default:					/** ���ܵĴ��� **/
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
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms���ʼ��ָ�ƴ�����
    	MFPOWER_ON();				//ָ�ƴ���������
    	g_u8pswordNO = 0;
    	
		OLED_cleanPages(5, 7);		//������
		DISP_line3rd_delDoing(3);	//ɾ����...
		
		//vp_stor(CVOPID_Doing);	//����
    	//vp_stor(CVOPID_DEL);		//ɾ��
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CFIGER_INIT); //!!!!���û�б�
		
		g_u8pswordNO++;		//try counter
		if(g_u8pswordNO > 3) {
			goto GOTO_DELETEFAIL;
		} else {
			FP_INIT();
		}
		break;
		
	case CMSG_FGINIT:				//���ָ�ƴ�������ʼ��. ���Խ���ɾ����
    	g_u8pswordNO = 0;	
    	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_DeRegister(u16Temp, u16Temp);
		break;
		
	case CMSG_FGCOMFAIL:		//ͨ�Ź��ϣ�����һ��
    	u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
		FP_DeRegister(u16Temp, u16Temp);
		break;
		
	case CMSG_FGOPFAIL:
	GOTO_DELETEFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
    	
		func.func = f_mngFinger_delmngrFail;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_FGDERG:				//���ָ�ƴ�������ʼ��
		promptInit();
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
    	
		u16Temp = idxDeltaTab[g_u8menuNO] + g_u8pswordSel;
    	fingerTab_setEntry(u16Temp, CENTRYFLAG_IDLE);
    	
		func.func = f_mngFinger_delmngrSucc;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:					/** ���ܵĴ��� **/
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    	
		OLED_cleanPages(0, 7);		//(���)
		DISP_line3rd_delSucc(3);	//ɾ���ɹ�
		
		vp_stor(CVOPID_DEL);
		vp_stor(CVOPID_SUCESS);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//2���ص�ѡ��ID�Ľ׶�(IDѡ��-->����ִ��-->�˴�)
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//2���ص�ѡ��ID�Ľ׶�(IDѡ��-->����ִ��-->�˴�)
    	}
		break;
		
	default:					/** ���ܵĴ��� **/
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    	
		OLED_cleanPages(0, 7);		//(���)
		DISP_line3rd_delFail(3);	//ɾ��ʧ��
		
		vp_stor(CVOPID_DEL);
		vp_stor(CVOPID_FAILED);
		break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
		break;
		
	case CMSG_TMR:
		if(g_tick++ > 10) {
			promptInit();
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);	//�����ص�ѡ��ID�Ľ׶�
    	}
		break;
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * ѡ��һ��Ҫɾ�����û�ID��
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
    	if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)) {				//ָ�ƿ�
    		promptInit();
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_300MS, CMSG_INIT);	//׼��������һ�׶�
    		
			func.func = f_mngFinger_empty;
    		fstack_push(&g_fstack, &func);
    	} else {
			DISP_L3_menu_line1st_deluser();	//��һ��
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
			DISP_line3rd_menu_select();		//������
		}
		break;
	
	case CMSG_NEXT:
		
		break;
		
	case CMSG_TMR:
        //ledPrompt((g_tick%13), TIMER_100MS); 	//for test only        
		g_tick++;
		break;	
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}
    		
			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
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
    		if(i >= (CFUSER_MAX  - CFUSER_MIN + 1)){		//���û���ҵ���������������ԭ����
    			g_u8pswordSel = j;
    		}

			DISP_line2nd_menu_IDxxxSel(g_u8pswordSel, CFUSER_MAX);//�ڶ���
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
    		
    		//g_u8pswordSel = g_u8pswordSel;		//�û�ID��
			func.func = f_mngFinger_delmngrConfirm;
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CFUSER_MIN
#undef	CFUSER_MAX
}

/*******************************************************************************
 * ָ�ƿ�������ʾ
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
    		fstack_pop(&g_fstack);	//��2��
		}
		break;	
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//��2��
		break;
			
	default:					/** ���ܵĴ��� **/
		break;	
	}
	return  0;
}

/*******************************************************************************
 * ָ����������ʾ
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
    		fstack_pop(&g_fstack);	//��2��
		}
		break;	
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	
    	fstack_pop(&g_fstack);
    	fstack_pop(&g_fstack);	//��2��
		break;
			
	default:					/** ���ܵĴ��� **/
		break;	
	}
	return  0;
}

/////////////////////////////////////////////////////

