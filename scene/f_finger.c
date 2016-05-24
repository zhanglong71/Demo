
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

u16 g_u16FPid[3];	//����+��һ��+�ڶ���

static u8 u8SearchCount = 0;	/** �ȶԴ��� **/
/*******************************************************************************
 * local function
 * function: ������Դ���ȴ������ȶ�
 * 
 *******************************************************************************/
int f_finger(unsigned *pMsg)
{
    func_t func;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:
		if(g_tick++ >= 5) {		//�������ų�ʱ
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
        	MFPOWER_ON();					//ָ�ƴ���������
        	
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        	promptDelay(TIMER_100MS);		//�ȴ�---�����ȶ�����ָ�ƴ��������ͳ�ʼ������
        	MFPid_CLEAN("���FPid");
    	}
    	adcSample_Start(&g_adcData);		//��ѹ������ʼ
		break;
		
	case CMSG_SECO:			//�ٴν���ָ�Ʊȶ�״̬
		MFPOWER_ON();					//ָ�ƴ���������
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        //promptDelay(TIMER_100MS);		//�ȴ�---�����ȶ�����ָ�ƴ��������ͳ�ʼ������
        
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
 * ��ʼ��
 *******************************************************************************/
int f_fingerInit(unsigned *pMsg)
{
  	//int i;
    func_t func;

    switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_TMR:
		if(g_tick++ >= 3) {		//3s�ڳ�ʼ��ʧ��(ָ�ƴ���������)
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
	case CFIGER_ON:		/** ��⵽ָ�ư��� **/
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
		
	case CFIGER_ON:		/** ��⵽ָ�ư��� **/
        //ledPrompt(11, TIMER_100MS);
		break;
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * finger search
 * Description: ִ�в�ѯ����
 *******************************************************************************/
int f_fingerSearch(unsigned *pMsg)
{
    func_t func;
    //int i;

    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_TMR:    
		g_tick++;
		if(g_tick > 10)	/** ��ʱ **/
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
		FP_Query(0, MUSER_ID_MAX - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		//FP_Query(0, CTOTALFINGER - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
        u8SearchCount = 0;
		break;
		
	case CMSG_FGQUERY:
		/** ��֤ͨ�� **/
		MFPOWER_OFF();					/** ָ��ͷ�ϵ� **/
		g_tick = 0;						/** ��ֹ���ܵĲ���;�У���ʱ�˳� **/
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	/** ��ʱ **/
    	
    	fstack_init(&g_fstack);
		func.func = f_fingerSearchSuccess;
    	fstack_push(&g_fstack, &func);
		break;	
		
	case CMSG_FGDOING:					/** �������ݴ� **/
	case CMSG_FGQUERYING:				/** ����������, ��ͼ���������� **/
	case CMSG_FGCOMFAIL:				/** ͨ��У����������� **/
		FP_Query(0, MUSER_ID_MAX - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		//FP_Query(0, CTOTALFINGER - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		break;
		
	case CMSG_FGOPFAIL:
		if(++u8SearchCount < 3) {
			FP_Query(0, MUSER_ID_MAX - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
			//FP_Query(0, CTOTALFINGER - 1);	/** �����ָ��ģ�壬����ָ�Ʊȶ����� **/
		} else {
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);	/** ��ʱ **/
			fstack_init(&g_fstack);
			func.func = f_fingerSearchFail;
    		fstack_push(&g_fstack, &func);
		}
		break;
		
	case CFIGER_ON:		/** ��⵽ָ�ư��� **/
        //LEDall_off();
		break;
			
	default:
		break;
	}  

    return  0;
}

/*******************************************************************************
 * f_fingerSearchSuccess
 * Description: ��ѯ�ɹ����ж��Ƿ�Ҫִ�п�������
 * 
 * ��Ҫ���ǵĳ���
 * ����1��"ָ��/����"ģʽ(����)
 * ����2��"ָ��+ָ��"ģʽ(ֻ��һöָ�ƣ�(Display + VOP)����)
 * ����2��"ָ��+ָ��"ģʽ(����һöָ�ƣ�(Display + VOP)Ȼ��ȴ�������һ����֤)
 * ����3��"ָ��+ָ��"ģʽ(��öָ�ƶ�ͨ������öָ�Ʋ�ͬ��(Display + VOP)����)
 * ����4��"ָ��+ָ��"ģʽ(��öָ�ƶ�ͨ������öָ����ͬ��(Display + VOP)��֤ʧ��)
 * ����5��"ָ��+����"ģʽ(�����룺(Display + VOP)����)
 * ����6��"ָ��+����"ģʽ(�����룺(Display + VOP)�ȴ�������һ����֤)
 * 
 * CMSG_INIT
 * 	1. ��ʾͨ����֤�����
 * 	2. ������ʾ����֤ͨ��
 * 	3. ������㿪��������ִ�п�������
 * 
 * CPMT_OVER
 * ������ʾ��ɺ�ִ���²�������
 *	1. ������㿪�������������Ѿ������ˣ�����Ҫ���κ�����(�˹�)
 *  2. ����ڵȴ���һ�������֤��ֱ�ӽ�����һ�֡�ע�ⷢ�ʵ�����Ϣ
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT��Ϣ **/
		/** voicePrompt **/
		promptInit();
    	promptDelay(TIMER_100MS);
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_PASSED);
    	/** OLED Display **/
    	OLED_cleanPages(0, 7);			//����
        if(MisADMIN(MFPid_TOP(0))) {
        	DISP_line2nd_PFadmin_ID(M2AdminID(MFPid_TOP(0)));
		} else {
			DISP_line2nd_PFuser_ID(M2UserID(MFPid_TOP(0)));
		}
		DISP_PSverify_line3rd_succ(1);
		
		/** ���Ƿ�ﵽ��������? **/
		if(isOpenMode_FPorPS()) {	//"ָ��/����"ģʽ
			/**
			 * �����ĳ���1��"ָ��/����"ģʽ��ָ����֤ͨ��
			 **/
		 
			/** ��¼�˿����¼� **/
			MRECORD_setRecordHead(x);
			MRECORD_setRecordLen(x);
			Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
			MRECORD_cleanRecordFlag(x);
			MRECORD_setRecordFPid(MFPid_TOP(0));
			record_write(recordIdxTab_getFreeRecordId());
			/** record table **/
			recordIdxTab_update();
		 
	CGOTO_OPENLOCK:					//����
			
    		vp_stor(CVOPID_Please);
    		vp_stor(CVOPID_TURNHANDLE);
			lockAction();	/** ���� **/
    		break;
    	} else if(isOpenMode_FPandFP()) {	//"ָ��+ָ��"ģʽ
    		if((fingerTab_getBusyEntryNum() < 2)) {
    			/**
				 * �����ĳ���2��"ָ��+ָ��"ģʽ������һöָ�ƣ�����֤ͨ��
			 	 **/
			 	 
				/** ��¼�˿����¼� **/
				MRECORD_setRecordHead(x);
				MRECORD_setRecordLen(x);
				Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
				MRECORD_cleanRecordFlag(x);
				MRECORD_setRecordFPid(MFPid_TOP(0));
				record_write(recordIdxTab_getFreeRecordId());
				/** record table **/
				recordIdxTab_update();
				
				goto	CGOTO_OPENLOCK;
    		} else {	//��¼�����öָ��
    			if(MFPid_GETLEN("ͨ����ָ�ƱȶԵĸ���") >= 2) {
    				if(MisFPid_NEQ("���αȶԵ��ǲ�ͬ��ָ��")) {
    					/**
			 			 * �����ĳ���3��"ָ��+ָ��"ģʽ����ö��ָͬ����֤ͨ��
			 			 **/
			 			/** ��¼�˿����¼� **/
						MRECORD_setRecordHead(x);
						MRECORD_setRecordLen(x);
						Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
						MRECORD_cleanRecordFlag(x);
						MRECORD_setRecordFPid(MFPid_TOP(1));
						MRECORD_setRecord2ndFPid(MFPid_TOP(0));
						record_write(recordIdxTab_getFreeRecordId());
						/** record table **/
						recordIdxTab_update();
				
						goto	CGOTO_OPENLOCK;
					} else {	//���αȶԵ�ָ����ͬ
						g_tick = 0;
						SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
						fstack_init(&g_fstack);
						func.func = f_fingerSearchFail;
    					fstack_push(&g_fstack, &func);
					}
    			} else {	//ֻ�ȶ���һöָ�ƣ��ٱȶ���һöָ��(������������CPMT_OVER�д���)
    			}
    		}
    	} if(isOpenMode_FPandPS()) {	//"ָ��+����"ģʽ
    		if(psword_getBusyEntryNum() <= 0) {
    			/**
			 	 * �����ĳ���4��"ָ��+����"ģʽ��û��¼������
			 	 **/
			 	 /** ��¼�˿����¼� **/
					MRECORD_setRecordHead(x);
					MRECORD_setRecordLen(x);
					Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
					MRECORD_cleanRecordFlag(x);
					MRECORD_setRecordFPid(MFPid_TOP(0));
					record_write(recordIdxTab_getFreeRecordId());
					/** record table **/
					recordIdxTab_update();
				
				goto	CGOTO_OPENLOCK;
    		} else {	//�ڲ������������ٽ���������֤
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
	
	case CPMT_OVER:		/** ����ǰ����ʾ����� **/	
		promptInit();
		/** ���Ƿ�ﵽ��������? **/
		if(isOpenMode_FPorPS()) {	//"ָ��/����"ģʽ
		/**
		 * �����ĳ���1���˴��޶���
		 **/
    	} else if(isOpenMode_FPandFP()) {	//"ָ��+ָ��"ģʽ
    		if((fingerTab_getBusyEntryNum() < 2)) {
    			/**
				 * �����ĳ���2��"ָ��+ָ��"ģʽ������һöָ�ƣ�����֤ͨ��(�޶���)
			 	 **/
    		} else {	//��¼�����öָ��
    			if(MFPid_GETLEN("ͨ����ָ�ƱȶԵĸ���") >= 2) {
    				if(MisFPid_NEQ("���αȶԵ��ǲ�ͬ��ָ��")) {
    					/**
			 			 * �����ĳ���3��"ָ��+ָ��"ģʽ����ö��ָͬ����֤ͨ��(�޶���)
			 			 **/
					} else {	//���αȶԵ�ָ����ͬ(����ִ�е�����)
						#if	0
						g_tick = 0;
						SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
						fstack_init(&g_fstack);
						func.func = f_fingerSearchFail;
    					fstack_push(&g_fstack, &func);
    					#endif
					}
    			} else {	//ֻ�ȶ���һöָ�ƣ��ٱȶ���һöָ��
					fstack_init(&g_fstack);
    				func.func = f_finger;
    				fstack_push(&g_fstack, &func);
    				
					g_tick = 0;	
					SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_SECO);
    			}
    		}
    	} if(isOpenMode_FPandPS()) {	//"ָ��+����"ģʽ
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
			 	 * �����ĳ���4��"ָ��+����"ģʽ��û��¼������(�޶���)
			 	 **/
    		}
    	}
		break;

	case CACT_OVER:		/** ������� **/	
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
 * Description: ��ѯʧ�ܣ��������˳�
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT��Ϣ **/
		
		promptInit();	
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_FAILED);
    	promptDelay(TIMER_100MS);
    	
    	OLED_cleanPages(0, 7);	//���
    	DISP_PSverify_line3rd_succ(0);
		break;
	
	case CPMT_OVER:		/** ��ʾ����� **/	
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
 * Description: ��ָ��ģ��
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT��Ϣ **/
		
		promptInit();	
		vp_stor(CVOPID_FORYOURSAFTY);	//Ϊ�����İ�ȫ
    	vp_stor(CVOPID_PLEASEPRESS);	//�밴
    	vp_stor(CVOPID_SET);			//����
    	vp_stor(CVOPID_Key);			//��
    	vp_stor(CVOPID_ADD);			//���
    	vp_stor(CVOPID_MANAGER);		//����Ա
    	vp_stor(CVOPID_FPRINT);			//ָ��
    	
    	OLED_cleanPages(0, 0);	//���
    	OLED_cleanPages(7, 7);	//���
    	//DISP_PSverify_line3rd_succ(0);
    	DISP_open_FPempty();
    	
		lockAction();	/** ���� **/
		break;
	
	case CPMT_OVER:		/** ����ǰ����ʾ����� **/	
		promptInit();
		break;

	case CACT_OVER:		/** ����ǰ����ʾ����� **/	
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

