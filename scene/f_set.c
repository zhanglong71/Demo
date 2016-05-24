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

/** ���ܱ�ע������Ҫ����ʾ˳��һ�£� ������ɻ��� **/
const static pfunc_t menuTab[] = {
	f_mngFinger,
	f_mngPsword,
	f_mngBluetooth,
	f_mngSystem,
	f_mngQuery,
};
/*******************************************************************************
 * �������ò˵���
 * ���������֤��¼���1����Աָ��
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
		if(fingerTab_isEmpty()) {	//û��ָ��ģ��, ¼��ָ��ģ��
    		fstack_init(&g_fstack);
			func.func = f_set_addSuperAdmin;
    		fstack_push(&g_fstack, &func);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		} else {					//��ָ��ģ�壬���������֤
			
			ClrTimer_irq(&g_timer[0]);
			/** display **/
        	OLED_cleanPages(3, 7);
    		DISP_line1st_authen_manager();	//��֤����Ա
    		DISP_menu_line2nd_PP(4);		//�밴��ָ
			
			/** voice prompt **/
			vp_stor(CVOPID_COMPARISON);
			vp_stor(CVOPID_MANAGER);
			vp_stor(CVOPID_Please);
			vp_stor(CVOPID_Press);
    		vp_stor(CVOPID_FINGER);
    		
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);
			//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_FGINIT);
    		MFPOWER_ON();				//ָ�ƴ���������
    		
        	MFPid_CLEAN("���FPid");
    	}
    	MisLOWPOWERCLEAN("��հ���");		//��հ���(���ù���)
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
	case CMSG_FGINIT:				//���ָ�ƴ�������ʼ��
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	g_u8pswordNO = 0;
    	FP_Query(0, MUSER_ID_BASE - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		break;
	case CMSG_FGDOING:					/** �������ݴ� **/
	case CMSG_FGQUERYING:				/** ���������� **/
	case CMSG_FGCOMFAIL:				/** ͨ��У����������� **/
		FP_Query(0, MUSER_ID_BASE - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		break;
		
	case CMSG_FGQUERY:					/** Note: the JP100-A not specified ID **/
		
		if((MFPid_GETid("��id��Χ") > 0) && (MFPid_GETid("��id��Χ") <= MUSER_ID_BASE)) {
    		promptInit();
			MFPOWER_OFF();					/** ָ��ͷ�ϵ� **/
			g_tick = 0;						/** ��ֹ���ܵĲ���;�У���ʱ�˳� **/
			SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_MATCH);	/** ��ʱ **/
    		
    		fstack_init(&g_fstack);
			func.func = f_set_authenSucc;
    		fstack_push(&g_fstack, &func);
    	} else {
    		goto CGOTO_AUTHEN_FAILED;
    	}
		break;	
		
	case CMSG_FGOPFAIL:		//��ʱ��û�鵽��ģ���
	CGOTO_AUTHEN_FAILED:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_MATCH);	/** ��ʱ **/
		
		fstack_init(&g_fstack);
		func.func = f_set_authenFail;
    	fstack_push(&g_fstack, &func);
		break;	
					
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_SET:				/** �������� **/
			break;
			
    	case CKEY_asterisk:
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		beep();
    		MFPOWER_OFF();		//��֤Ȩ��ʱ�����˳���
    		
			MPSWORD_PUSH(((msg_t *)pMsg)->msgValue);	//������ջ����ʲô�ã�����͵�У�顢����ģʽ��״̬�Ĳ�������	
    		if(MPSWORD_GETLEN("����") == 2)
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
			if(MisLOWPOWERCHECK("�͵�У��") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("�ָ���������") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ(why shutdown ?)
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
			break;
			
		default:					/** ���ܵĴ��� **/
			break;
		}
		break;

	default:
		break;
	}
    return  0;
}

/*******************************************************************************
 * ѡ��"ָ��/����/ϵͳ/��ѯ"����
 * ����Ա�����֤ͨ��/¼���һ����Ա�ɹ��������
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
		DISP_line1st_menu();			//��һ��
		DISP_L1_menu(g_u8menuNO);		//�ڶ���
		DISP_line3rd_menu_select();		//������
		
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
		
		/** ֻ����2, 8, *, # **/
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
		DISP_line1st_menu();			//��һ��
		DISP_L1_menu(g_u8menuNO);		//�ڶ���
		DISP_line3rd_menu_select();		//������
		
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
		
		/** ֻ����2, 8, *, # **/
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
	case CFIGER_ON:		/** ��⵽ָ�ư��� **/
		break;

	case CMSG_DKEY:							/** ��⵽�������� **/	
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
			
    	case CKEY_SET:				/** �������� **/
			ledPrompt(0, TIMER_100MS); 	//for test only
    		beep();
			break;
				
		default:					/** ���ܵĴ��� **/
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
 * ��һ�ε�¼����Ҫ¼���һ����Ա
 * ¼���һ������ǰ����������ʾ��ʾ
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
		DISP_L3_menu_line1st_addmanager();	//��ӹ���Ա
		DISP_menu_line2nd_PP(2);
		OLED_cleanPages(5, 7);		//������
		
		/** voice prompt **/
		vp_stor(CVOPID_ADD);
		vp_stor(CVOPID_MANAGER);
		vp_stor(CVOPID_Please);
		vp_stor(CVOPID_Press);
    	vp_stor(CVOPID_FINGER);
    	
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_SET:				/** �������� **/
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
			if(MisLOWPOWERCHECK("�͵�У��") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("�ָ���������") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
    		break;
    				
		default:					/** ���ܵĴ��� **/
			break;
		}
		break;		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/***********************************************************
 * ¼���һ����ָ�ƣ� 
 * �ϵ�-->initial-->eroll-->success/fail
 ***********************************************************/
int f_set_addSuperAdminDoing(unsigned *pMsg)
{	
	func_t func;
    
   	switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CFIGER_INIT);	//100ms���ʼ��ָ�ƴ�����(JP100-A����ʡ�Դ˲���)
		//SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_FGINIT);	//100ms���ʼ��ָ�ƴ�����
    	MFPOWER_ON();				//ָ�ƴ���������
    	
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
			
	case CFIGER_INIT:	//ʱʹ��JP100-A�ĳ�����ʡ�Դ˲��衣)
		FP_INIT();
		break;
		
	//case CMSG_FGCCEL:				//ȷ��ָ�ƴ�������ʼ��
	case CMSG_FGINIT:				//���ָ�ƴ�������ʼ��
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
    	g_u8pswordNO = 0;
		//FP_Register(0, 0, g_u8pswordNO);	//BLH3001��ָ��ģ����Ŵ�0��ʼ
		FP_Register(1, 1, g_u8pswordNO);	//JP100-A��ָ��ģ����Ŵ�1��ʼ
		break;
		
	case CMSG_FGOPFAIL:
		g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_ENROLL);
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
    	
		func.func = f_set_authenFail;
    	fstack_push(&g_fstack, &func);
		break;
			
	case CMSG_FGRGST:				//ָ��¼�����
		MFPOWER_OFF();				//ָ�ƴ������ϵ�
		func.func = f_set_authenSucc;
    	fstack_push(&g_fstack, &func);
    		
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CINIT_ENROLL);
    		
    	/** byteͼ��־ **/
    	//fingerTab_setEntry(0, CENTRYFLAG_BUSY);
    	fingerTab_setEntry(1, CENTRYFLAG_BUSY);
		break;
		
	case CMSG_FGRGSTING:		//ͼ�����������ڳ�������
	case CMSG_FGDOING:			//�������ݴ�
	case CMSG_FGCOMFAIL:		//ͨ��У�����������
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
		g_u8pswordNO++;	/** �µ�JP100-Aָ�ƴ������ƺ��ò��� **/		
		//FP_Register(0, 0, g_u8pswordNO);	//BLH3001��ָ��ģ����Ŵ�0��ʼ
		FP_Register(1, 1, g_u8pswordNO);	//JP100-A��ָ��ģ����Ŵ�1��ʼ
		
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
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
			if(MisLOWPOWERCHECK("�͵�У��") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
				func.func = f_set_lowPowerCali;
    			fstack_push(&g_fstack, &func);
			}
			
			if(MisRESTORE("�ָ���������") && SET_WAKECHECK("���ܼ�")) {
				MFPOWER_OFF();					//�ر�ָ�ƴ�������Դ
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    			
    			fstack_init(&g_fstack);
    			func.func = f_mngRestoreFactory;
    			fstack_push(&g_fstack, &func);
			}
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
 * �ɹ�����˵�����
 * ��������
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    
		//DISP_line2nd_PFadmin_ID(M2AdminID(0));    
		DISP_line2nd_PFadmin_ID(M2AdminID(1));
		DISP_line3rd_AddSucc();
	
		vp_stor(CVOPID_ADD);
		vp_stor(CVOPID_SUCESS);
    	break;
    		
	case CINIT_MATCH:					/** ͨ������Ա��֤ **/
		promptInit();
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
		
		OLED_cleanPages(0, 2);			//������    
		DISP_line2nd_PFadmin_ID(M2AdminID(MFPid_TOP(0)));
		DISP_PSverify_line3rd_succ(1);
    	break;
		
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		/** ��������ջ **/
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
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	//100ms���ʼ��ָ�ƴ�����
    	}
		break;
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/***********************************************************
 * ���/��֤��1����Աʧ��
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
    	
		OLED_cleanPages(2, 2);		//������
		OLED_cleanPages(5, 7);		//������
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//���
		vp_stor(CVOPID_FAILED);		//ʧ��
		break;
		#endif
		
	case CINIT_ENROLL:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    	
		OLED_cleanPages(2, 2);		//������
		OLED_cleanPages(5, 7);		//������
		DISP_line3rd_AddFail();
		
		vp_stor(CVOPID_ADD);		//���
		vp_stor(CVOPID_FAILED);		//ʧ��
    	break;
    		
	case CINIT_MATCH:					/** ͨ������Ա��֤ **/
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);	//100ms���ʼ��ָ�ƴ�����
    
		DISP_PSverify_line3rd_succ(0);
		
		vp_stor(CVOPID_COMPARISON);	//��֤
    	vp_stor(CVOPID_FAILED);		//ʧ��
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
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}


/*******************************************************************************
 * �͵�У׼
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
    	adcSample_Start(&g_adcData);	//����������ʱ
    	
    	OLED_cleanPages(0, 0);
    	DISP_lowPowerCheck();			//�͵�У׼
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//���Ե�...
		break;
					
	case CMSG_TMR:
		if(adcSample_Out(&g_adcData,  &temp1) == TRUE) {		/** ADC��ѹ�������, ȡ��ֵ **/
			adcSample_Stop(&g_adcData);
			VOL_setRefValue(temp1);
			DISP_menu_MNGSYS_line3rd_succ(5);
			
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    		
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		
		if(g_tick++ >= 20) {	//��ʱ
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		break;
	default:					/** ���ܵĴ��� **/
		break;
	}	
	return	0;	
}

/*******************************************************************************
 * ��������
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
    	adcSample_Start(&g_adcData);	//����������ʱ
    	
    	OLED_cleanPages(0, 0);
    	//DISP_lowPowerCheck();			//�͵�У׼
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//���Ե�...
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
		if(g_tick++ >= 20) {	//��ʱ
    		g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
			
			fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		break;
	default:					/** ���ܵĴ��� **/
		break;
	}	
	return	0;	
}


/*******************************************************************************
 * ������ʱ. ׼����ʼ������ϵͳ
 *******************************************************************************/
int f_set_longPress(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		
    	OLED_cleanPages(0, 0);
    	//DISP_lowPowerCheck();			//�͵�У׼
    	OLED_cleanPages(5, 5);
    	DISP_menu_MNGSYS_line3rd_Waiting(6);	//���Ե�...
    	
		break;
			
	case CPMT_OVER:
		promptInit();
		break;
						
	case CMSG_TMR:
		if(g_tick++ >= 10) {	//��ʱ
    		promptInit();
    		beep();
    		
			fstack_init(&g_fstack);
			func.func = f_mngRestoreFactory;
    		fstack_push(&g_fstack, &func);
    		
    		//g_u8pswordNO++;
    		//if(g_u8pswordNO == 1) {		//��n�ΰ���#��
	    		OLED_cleanPages(1, 1);
				DISP_menu_MNGSYS_line2nd_RestoreDoing();	//�ָ�����
				//DISP_menu_MNGSYS_line3rd_RestoreDoing(6);
		    	OLED_cleanPages(5, 5);
				DISP_menu_MNGSYS_line3rd_Waiting(6);	//���Ե�
    			MFPOWER_ON();				//ָ�ƴ���������
    			
				psword_delAll();	//ȫ������
				//psword_delAll();	//ȫ�������豸
				fingerTab_clean();	//ָ��������
				
				record_clean();		//������¼
				
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_300MS, CFIGER_INIT);
    		//}
		}
		break;
		
	case CMSG_UKEY:			/** ��⵽�����ɿ� **/
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:				/** �������� **/
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
	default:					/** ���ܵĴ��� **/
		break;
	}	
	return	0;	
}
/////////////////////////////////////////////////////

