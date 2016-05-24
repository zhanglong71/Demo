#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

//#include "led.h"

#include "arch.h"
#include "driver.h"

#include "l_i2c_datetime.h"
#include "l_queryRecord.h"
#include "l_fingprint.h"
#include "l_oled.h"
#include "f_set.h"
#include "f_idle.h"
#include "f_mngSystem.h"

const static pfunc_t mngSystemTab[] = {
	f_mngOpenMode,
	f_mngDatetime,
	f_mngRestoreFactory,
};
/*******************************************************************************
 * ϵͳ����
 *******************************************************************************/
int f_mngSystem(unsigned *pMsg)
{
#define	CSYS_MIN	0
#define	CSYS_MAX	(MTABSIZE(mngSystemTab) - 1)
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		break;

	case CMSG_INIT:	
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        g_u8menuNO = CSYS_MIN;
        
		DISP_menu_MNGSYS_line1st();		//��һ��
		DISP_L2_menu_sysmng(g_u8menuNO);	//�ڶ���
		OLED_cleanPages(5, 5);
		DISP_line3rd_menu_select();		//������
		break;	
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, CSYS_MAX, CSYS_MIN);
    		DISP_L2_menu_sysmng(g_u8menuNO);	//�ڶ���
			break;
		
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS); 	//for test only
    		beep();
    		g_u8menuNO = value_inc(g_u8menuNO, CSYS_MAX, CSYS_MIN);
    		DISP_L2_menu_sysmng(g_u8menuNO);	//�ڶ���
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
    		
    		func.func = mngSystemTab[g_u8menuNO];
    		fstack_push(&g_fstack, &func);
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:						/** ���ܵĴ��� **/
		break;
	}
	return	0;
	
#undef	CSYS_MIN
#undef	CSYS_MAX
}

/*******************************************************************************
 * �ָ�����ֵ
 *******************************************************************************/
int f_mngRestoreFactory(unsigned *pMsg)
{
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:	
		OLED_cleanPages(0, 1);						//��1��
		DISP_menu_MNGSYS_line2nd_RestoreConfirm();	//�ָ�������
		DISP_line3rd_menu_confirm(6);		/** (��(*)   ��(#) ) **/
		g_u8pswordNO = 0;
		break;
	
	case CPMT_OVER:
		promptInit();
		break;
		
	case CFIGER_INIT:
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_300MS, CFIGER_INIT);
		FP_INIT();
		break;
	
	case CMSG_FGINIT:				//���ָ�ƴ�������ʼ��. ���Խ���ɾ����
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_3SEC, CMSG_FGINIT);
		
    	g_u8pswordNO = 0;
		//MFP_DelAll();		//ɾ��ȫ��ָ��
		FP_DelAll();	//ɾ��ȫ��ָ��
		break;
	
	case CMSG_FGOPFAIL:
		/** ������ȫ��ָ��ʧ�ܣ���ζ��ʲô? ����ô��? **/
		break;
						
	case CMSG_FGDERG:				//���ָ��ģ��ɾ��
		promptInit();
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	MFPOWER_OFF();				//ָ�ƴ������ϵ�
    	
		func.func = f_mngRestoreFactoryDone;
    	fstack_push(&g_fstack, &func);
		break;
		
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_INIT);
    		fstack_pop(&g_fstack);
			break;
			
    	case CKEY_pound:
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		beep();
    		g_u8pswordNO++;
    		if(g_u8pswordNO == 1) {		//��n�ΰ���#��
				DISP_menu_MNGSYS_line2nd_RestoreDoing();	//�ָ�����
				//DISP_menu_MNGSYS_line3rd_RestoreDoing(6);
				OLED_cleanPages(5, 5);
				DISP_menu_MNGSYS_line3rd_Waiting(6);	//���Ե�
    			MFPOWER_ON();				//ָ�ƴ���������
    			
				psword_delAll();			//ȫ������
				bluetoothEntry_delAll();	//ȫ�������豸
				fingerTab_clean();	//ָ��������
				
				record_clean();		//������¼
				
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_300MS, CFIGER_INIT);
    		}
			break;
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/***********************************************************/
int f_mngRestoreFactoryDone(unsigned *pMsg)
{
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		break;

	case CMSG_INIT:	
		//DISP_menu_MNGSYS_line2nd_RestoreDoing();
		DISP_menu_MNGSYS_line3rd_succ(5);

		vp_stor(CVOPID_RESTORE);
    	vp_stor(CVOPID_SUCESS);
    			
    	g_tick = 0;
    	SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
		break;
	case CMSG_TMR:
		if(g_tick++ >= 10) {
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		
    		fstack_init(&g_fstack);
			func.func = f_idle;
    		fstack_push(&g_fstack, &func);
		}
		break;
	
	case CPMT_OVER:
		promptInit();
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
		
    	fstack_init(&g_fstack);
		func.func = f_idle;
    	fstack_push(&g_fstack, &func);
		break;
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
}

/*******************************************************************************
 * ������ʽ
 *******************************************************************************/
int f_mngOpenMode(unsigned *pMsg)
{
#define	COPENMODE_MIN	0
#define	COPENMODE_MAX	2
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        g_u8menuNO = COPENMODE_MIN;
        
        All_Screen(0);
		DISP_menu_MNGSYS_line1st_OpenMode();	//��һ��
		DISP_L2_menu_OpenModemng(g_u8menuNO);	//�ڶ���
		OLED_cleanPages(5, 5);
		DISP_line3rd_menu_select();				//������
		break;
				
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
    	case CKEY_2:
    		ledPrompt(2, TIMER_100MS);
    		promptInit();
    		beep();
    		g_u8menuNO = value_dec(g_u8menuNO, COPENMODE_MAX, COPENMODE_MIN);
    		DISP_L2_menu_OpenModemng(g_u8menuNO);
			break;
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		g_u8menuNO = value_inc(g_u8menuNO, COPENMODE_MAX, COPENMODE_MIN);
    		DISP_L2_menu_OpenModemng(g_u8menuNO);
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
    		
    		OpenMode_setValue(g_u8menuNO);
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		func.func = f_mngOpenModeDone;
    		fstack_push(&g_fstack, &func);
			break;	
            
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:					/** ���ܵĴ��� **/
		break;
	}
	return	0;
#undef	COPENMODE_MIN
#undef	COPENMODE_MAX
}
/*************************************************/
int f_mngOpenModeDone(unsigned *pMsg)
{
	//func_t func;
	#if	1
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CACT_OVER:
		break;

	case CMSG_INIT:	
		DISP_menu_MNGSYS_line3rd_succ(5);
    	vp_stor(CVOPID_SET);
    	vp_stor(CVOPID_SUCESS);
    	
    	
    	DISP_L2_menu_OpenModemng(OpenMode_getValue());
    		
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
		
	default:					/** ���ܵĴ��� **/
		break;
	}
	#endif
	return	0;
}

/*******************************************************************************
 * ���ɵ���ֵ�ı߽硣��[min, max, idxOfmem, idxOfPCF, mask]�ķ�ʽ��ʾ
 *
 * idxOfmem��idxOfPCF�Ķ�Ӧ��ϵ���£�
 * PCF8563�ĵ�ַΪ2,3,4,5,6,7,8(��Ӧ�룬�֣�ʱ���գ����ڣ��£���)
 * ���������δ����һ������g_u8password��������Ϊ0,1,2,3,4,5,6��λ��
 * 
 * �磺������仯��Χ�Ǵ�1��31, ��Ӧ����g_u8password��������3��ȡPCF8563�ĵ�ַΪ5�����ݣ�
 * 
 * ������mask��ԭ������Ϊ���֣�����ʱ��ʾ����, ���Է��ֶ���������(��λ)����
 *******************************************************************************/
  #pragma location="boundary_datetime"
__root __far const u8  boundary_datetime[][5] = {
	{0, 99, 6, 8, 0xff},	//��
	{1, 12, 5, 7, 0x1f },	//��
	{1, 31, 3, 5, 0x3f},	//��
	{0, 23, 2, 4, 0x3f},	//ʱ
	{0, 59, 1, 3, 0x7f},	//��
	{0, 59, 0, 2, 0x7f},	//��
	{0, 6,  4, 6, 0x07},	//����
};

#define	MDATETIME_STORE(index, value)	do{g_u8password[boundary_datetime[index][2]] = value;}while(0)
#define	MDATETIME_GET(index, value)	do{value = g_u8password[boundary_datetime[index][2]];}while(0)

#define	MDATETIME_COOKED(index)	do{g_u8password[boundary_datetime[index][2]] &= boundary_datetime[index][4];}while(0)
/*******************************************************************************
 * ����ʱ������
 *******************************************************************************/
int f_mngDatetime(unsigned *pMsg)
{
#define	CDATETIME_MIN	0
#define	CDATETIME_MAX	(MTABSIZE(boundary_datetime) - 1)
	func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);
		//OLED_cleanPages(5, 5);		
		MDATETIME_READ("����ʱ�䣬���뵽g_u8password[]��");	
        g_u8menuNO = CDATETIME_MIN;			//��ֵ����   
            
    	MDATETIME_COOKED(g_u8menuNO);
    	MDATETIME_GET(g_u8menuNO, g_u8pswordSel);
        g_u8pswordSel = MBCD2HEX(g_u8pswordSel);
        
		DISP_GetOrSetDateTime(1);
		DISP_line3rd_menu_select();				//������
		break;
			
	case CMSG_TMR:
		//SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        DISP_SetDateTime_Blink(g_u8menuNO, (g_tick & 0x01), MHEX2BCD(g_u8pswordSel));
        
		g_tick++;
		if(g_tick > 10) {
			promptInit();
    		
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
    		
    		g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			
    		g_u8pswordSel = value_dec(g_u8pswordSel, boundary_datetime[g_u8menuNO][1], boundary_datetime[g_u8menuNO][0]);
    		DISP_SetDateTime_Blink(g_u8menuNO, 0, MHEX2BCD(g_u8pswordSel));
			break;
			
    	case CKEY_8:
    		ledPrompt(8, TIMER_100MS);
    		promptInit();
    		beep();
    		
    		g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			
    		g_u8pswordSel = value_inc(g_u8pswordSel, boundary_datetime[g_u8menuNO][1], boundary_datetime[g_u8menuNO][0]);
    		DISP_SetDateTime_Blink(g_u8menuNO, 0, MHEX2BCD(g_u8pswordSel));
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
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			
    		MDATETIME_STORE(g_u8menuNO, MHEX2BCD(g_u8pswordSel));
    		DISP_SetDateTime_Blink(g_u8menuNO, 0, MHEX2BCD(g_u8pswordSel));
    		datetime_ByteWrite(boundary_datetime[g_u8menuNO][3], MHEX2BCD(g_u8pswordSel));
    		
    		g_u8menuNO++;
    		if(g_u8menuNO < CDATETIME_MAX) {
    			MDATETIME_COOKED(g_u8menuNO);
				MDATETIME_GET(g_u8menuNO, g_u8pswordSel);
        		g_u8pswordSel = MBCD2HEX(g_u8pswordSel);
        		DISP_SetDateTime_Blink(g_u8menuNO, 0, MHEX2BCD(g_u8pswordSel));			
    		} else {
    			MDATETIME_WRITE("д�뵽оƬ");
    			
    			g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    			
    			func.func = f_mngDatetimeDone;
    			fstack_push(&g_fstack, &func);
    		}
			break;	
            
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:						/** ���ܵĴ��� **/
		break;
	}
	return	0;
#undef	CDATETIME_MIN
#undef	CDATETIME_MAX
}


int f_mngDatetimeDone(unsigned *pMsg)
{
	//func_t func;
	
    switch(((msg_t *)pMsg)->msgType) 
	{
	case CMSG_INIT:
		g_tick = 0;
		SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
        
        All_Screen(0);	
		MDATETIME_READ("����ʱ�䣬���뵽g_u8password[]��");	
        //g_u8menuNO = CDATETIME_MIN;			//��ֵ����   
    	//MDATETIME_GET(g_u8menuNO, g_u8pswordSel);
        //g_u8pswordSel = MBCD2HEX(g_u8pswordSel);
        
		DISP_GetOrSetDateTime(1);
		//DISP_line3rd_menu_select();				//������
		OLED_cleanPages(6, 7);	
		break;
			
	case CMSG_TMR:
		if(g_tick++ > 3) {
    		promptInit();
    		goto CGOTO_DATETIME_DONE;
		}
		break;
				
	case CMSG_DKEY:							/** ��⵽�������� **/		
		switch(((msg_t *)pMsg)->msgValue)
		{	
		case CKEY_asterisk:
    		ledPrompt(10, TIMER_100MS);
    		promptInit();
    		beep();
    		goto CGOTO_DATETIME_DONE;
			break;
		
    	case CKEY_pound:	
    		ledPrompt(12, TIMER_100MS);
    		promptInit();
    		beep();
    	CGOTO_DATETIME_DONE:
    		g_tick = 0;
    		SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    		fstack_pop(&g_fstack);
    		fstack_pop(&g_fstack);
			break;	
            
		default:					/** ���ܵĴ��� **/
			break;
		}
	default:						/** ���ܵĴ��� **/
		break;
	}
	return	0;
#undef	CDATETIME_MIN
#undef	CDATETIME_MAX
}
/////////////////////////////////////////////////////

