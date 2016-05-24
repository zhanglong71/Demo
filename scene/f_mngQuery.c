#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

#include "arch.h"
#include "driver.h"

#include "l_fingprint.h"
#include "l_oled.h"
#include "l_i2c_datetime.h"
#include "l_queryRecord.h"
#include "f_set.h"
#include "f_idle.h"
#include "f_mngQuery.h"
#include "f_LowPowerAlarm.h"


const static pfunc_t mngQueryTab[] = {
	f_mngQueryRecord,
	f_mngQueryDatetime,
	f_mngQueryVersion,
};

/*******************************************************************************
 * ϵͳ��ѯ
 *******************************************************************************/
int f_mngQuery(unsigned *pMsg)
{
#define	CQUERY_MIN	0
#define	CQUERY_MAX	(MTABSIZE(mngQueryTab) - 1)
	func_t func;
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
		g_u8menuNO = CQUERY_MIN;
		OLED_cleanPages(0, 7);
		
		DISP_menu_Query_line1st();		//��һ��
		DISP_L2_menu_mngQuery(g_u8menuNO);
		DISP_line3rd_menu_select();		//������
		
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		break;	
	
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;	
			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_BACK);
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
    		g_tick = 0;
    		
    		g_u8menuNO = value_dec(g_u8menuNO, CQUERY_MAX, CQUERY_MIN);
    		//OLED_cleanPages(0, 7);
    		DISP_L2_menu_mngQuery(g_u8menuNO);	//�ڶ���
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		g_tick = 0;
    		
    		g_u8menuNO = value_inc(g_u8menuNO, CQUERY_MAX, CQUERY_MIN);
    		//OLED_cleanPages(0, 7);
    		DISP_L2_menu_mngQuery(g_u8menuNO);	//�ڶ���
			break;
			
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS); 	//for test only
    		promptInit();
    		beep();
    		
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
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    		
    		func.func = mngQueryTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
		break;
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CQUERY_MIN
#undef	CQUERY_MAX
}

/*******************************************************************************
 * �鿪����¼
 * ��ѯԭ�����£�
 * ��ʼֵ��ȡ�������е�ָ��FIRSTֵ
 * ����ֵ��ȡ1
 * ����ֵ��ȡ�������е�ָ��LENֵ
 * ���±߽磺����ѭ����ʾ
 *******************************************************************************/
int f_mngQueryRecord(unsigned *pMsg)
{
	//func_t func;
	
    switch(((msg_t *)pMsg)->msgType)
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		
        //record_read(0);				//read index table from flash
        if((recordIdxTab_getRecordCnt() != 0) 
        		&& (recordIdxTab_isIdxTabValid())) {
        	
            g_u8pswordSel =  recordIdxTab_getBusyRecordId();
            if(g_u8pswordSel != 0) {
        		record_read(g_u8pswordSel);	//read record data from flash
        		DISP_RecordId(g_u8pswordSel, MRECORD_getRecordAddr(x));	//��һ��
        		DISP_DateTime(MRECORD_getRecordAddr(x));				//�ڶ���
        		DISP_line3rd_menu_select();								//������
        	} else {
        	}
    	} else {
        	g_u8pswordSel = 0;
        	OLED_cleanPages(0, 1);
        	DISP_NoRecord();
        	OLED_cleanPages(5, 7);
    	}
		break;
				
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
		} else {
			if((recordIdxTab_getRecordCnt() == 0)
               || (recordIdxTab_isIdxTabValid() == 0)) {
				g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			fstack_pop(&g_fstack);
			}
		}
		break;
		
	case CPMT_OVER:		/** ��ʾ����� **/	
		promptInit();
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		promptInit();
    		beep();
    		g_tick = 0;
    		
    		if((recordIdxTab_getRecordCnt() != 0)
               && (recordIdxTab_isIdxTabValid())) {
               	
               	g_u8pswordSel = value_inc(g_u8pswordSel, recordIdxTab_getRecordCnt(), 1);
               	record_read(g_u8pswordSel);		//read record data from flash
        		DISP_RecordId(g_u8pswordSel, MRECORD_getRecordAddr(x));	//��һ��
        		DISP_DateTime(MRECORD_getRecordAddr(x));				//�ڶ���
        		//DISP_line3rd_menu_select();								//������
    		}
			break;
			
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		g_tick = 0;
    		
    		if((recordIdxTab_getRecordCnt() != 0)
               && (recordIdxTab_isIdxTabValid())) {
               	
               	g_u8pswordSel = value_dec(g_u8pswordSel, recordIdxTab_getRecordCnt(), 1);
               	record_read(g_u8pswordSel);		//read record data from flash
        		DISP_RecordId(g_u8pswordSel, MRECORD_getRecordAddr(x));	//��һ��
        		DISP_DateTime(MRECORD_getRecordAddr(x));				//�ڶ���
        		//DISP_line3rd_menu_select();								//������
    		}
			break;
			
		case CKEY_asterisk:		/** �˻� **/
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
    		
			break;
			
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * ��ʱ������
 *******************************************************************************/
int f_mngQueryDatetime(unsigned *pMsg)
{
	//func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
        #if	1
			//Mdatetime_read(MRECORD_getDataAddr("datetime"));
			//DISP_GetOrSetDateTime(0/** get and display datetime **/);
			
			Mdatetime_read(MRECORD_getDataAddr("datetime"));
			DISP_DateTime(MRECORD_getRecordAddr("datetime"));
        #else
    		MDATETIME_READ("����ʱ�䣬���뵽g_u8password[]��");
			DISP_GetOrSetDateTime(0/** get and display datetime **/);
		#endif
		break;
				
	case CMSG_TMR:
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
    		ledPrompt(2, TIMER_100MS);
    		promptInit();
    		beep();
    		
			break;
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		
			break;
			
		case CKEY_asterisk:		/** �˻� **/
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
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * ��汾
 *******************************************************************************/
int f_mngQueryVersion(unsigned *pMsg)
{
	//func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
		DISP_version();
		break;
				
	case CMSG_TMR:
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
    		ledPrompt(2, TIMER_100MS);
    		promptInit();
    		beep();
    		
			break;
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		
			break;
			
		case CKEY_asterisk:		/** �˻� **/
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
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/////////////////////////////////////////////////////

