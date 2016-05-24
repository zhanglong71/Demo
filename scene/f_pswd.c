
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

u8 g_u8pswordSel;	/** ¼����ʱ��ѡ��Ҫ¼�������ı��(�ڼ�������) **/
u8 g_u8pswordNO;	/** ¼����ʱ����¼�ǵڼ���¼�������(�ڼ���¼������룬Ҳ���ǰ���#�ŵĴ���) **/
u8 g_u8pswordLen;	/** ¼����ʱ����¼����ĳ���(����#�ź󣬼�¼��ǰ¼��ĳ���) **/
/*******************************************************************************
 * g_u8password[] ������;
 * 1. ��������
 * 2. ����¼�뼰ȷ��
 * 3. PCF8563ʱ������
 * 4. ������¼��ѯ
 *******************************************************************************/
u8 g_u8password[CPSWORD_MAXLEN * 2];	//������󳤶�����Ŀռ�(ע: 1.�͵�У�������˴˿ռ䡣 2. PCF8563ʱ������ʹ�ô˿ռ���Ϊ���ݻ���)

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
	
	case CMSG_INIT:		/** ����״̬��������ȶ�״̬ **/
        MFPid_CLEAN("���FPid");     
		goto	CGOTO_PSWD_FUNC;
	case CMSG_SECO:		//ָ����֤ͨ����,��������ȶ�״̬(��ֱ�ӽ������뿪��״̬�Ĳ�֮ͬ���������FPid)
		
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
			
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_SET:					/** �������� **/
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
    		//g_tick = 0;		//�����㣬��ֹ����ĳ�ʱ�䰴��
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
    		
    		//bluetooth_PWRoff();			//�رտ��ܵ�����ģ���Դ
        	//bluetooth_ATdisable();
        
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		#if	1
    		//g_u8pswordSel = psword_Query(g_u8password);
    		//g_u8pswordSel = psword_Query_00(g_u8password);
			//g_u8pswordSel = psword_QueryTab[0](g_u8password)
			
    		if(isMmgPswdEnable()) {	//������λ���빦������?
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
    			
		default:					/** ���ܵĴ��� **/
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
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);		/** CMSG_INIT��Ϣ **/
		
		promptInit();	
		vp_stor(CVOPID_COMPARISON);
    	vp_stor(CVOPID_FAILED);
    	promptDelay(TIMER_100MS);
    	
    	OLED_cleanPages(0, 7);	//���
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
		
	case CPMT_OVER:		/** ��ʾ����� **/	
		promptInit();
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
					
	default:					/** ���ܵĴ��� **/
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
    	
    	if(isOpenMode_FPorPS()) {	//"ָ��/����"ģʽ
		/**
		 * �����ĳ���1��"ָ��/����"ģʽ��ָ����֤ͨ��
		 **/
		 /** ��¼�˿����¼� **/
			MRECORD_setRecordHead(x);
			MRECORD_setRecordLen(x);
			Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
			MRECORD_cleanRecordFlag(x);
			MRECORD_setRecordPWid(g_u8pswordSel);
			record_write(recordIdxTab_getFreeRecordId());
			/** record table **/
			recordIdxTab_update();
		CGOTO_OPENLOCK:					//����
			
			vp_stor(CVOPID_COMPARISON);
    		vp_stor(CVOPID_PASSED);
    		vp_stor(CVOPID_Please);
    		vp_stor(CVOPID_TURNHANDLE);
    		
			lockAction();				/** ���� **/
			
    		OLED_cleanPages(0, 7);			//����
			DISP_line2nd_PSword_ID(g_u8pswordSel);
			DISP_PSverify_line3rd_succ(1);	
			
			g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    		break;
    	} else if(isOpenMode_FPandPS()) {	//"ָ��+����"ģʽ
			//if((MFPid_GETLEN("ָ�ƱȶԹ�һ��")) && (MFPid_GETid("ָ�Ʊȶ���Ч"))) {
			if(MFPid_GETLEN("ָ�ƱȶԹ�һ��")) {
				 /** ��¼�˿����¼� **/
				MRECORD_setRecordHead(x);
				MRECORD_setRecordLen(x);
				Mdatetime_read(MRECORD_getDataAddr("ȡ������ʽ��ʼ�Ĳ���")); 	//ʱ��д��������
				MRECORD_cleanRecordFlag(x);
				MRECORD_setRecordFPid(MFPid_TOP(0));
				MRECORD_setRecordPWid(g_u8pswordSel);
				record_write(recordIdxTab_getFreeRecordId());
				/** record table **/
				recordIdxTab_update();
				goto	CGOTO_OPENLOCK;	
			} else {	
				/***************************************************************
				 * (ָ��+����)ģʽ��ȱ��ָ�Ʊȶ�. Ӧ����ת����ָ֤�Ƶ�������ȥ
				 * 
				 ***************************************************************/
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
				fstack_init(&g_fstack);
				func.func = f_pswd_fail;
    			fstack_push(&g_fstack, &func);
			}
    	} else {		
    		//�쳣����(�Ȳ���ָ��+���룬�ֲ���ָ��/���룬������ʲôģʽ��?)
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
    	
		lockAction();	/** ���� **/
		
    	OLED_cleanPages(0, 7);			//����
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
		
	case CPMT_OVER:		/** ��ʾ����� **/	
		promptInit();
		break;
		
	case CACT_OVER:		/** ����ǰ����ʾ����� **/	
		actionInit();
			
		g_tick = 0;	
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
		fstack_init(&g_fstack);
    	func.func = f_LowPowerAlarm;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:					/** ���ܵĴ��� **/
		break;	
	}
	return  0;	
}
/////////////////////////////////////////////////////

