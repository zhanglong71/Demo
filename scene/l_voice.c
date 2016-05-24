
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "arch.h"
#include "driver.h"
#include "l_voice.h"

#define	MPLUSE_DATAWAIT(x)	do{delaySpin(24);}while(0)
#define	MDLUSE_RESETWAIT(x)	do{delaySpin(48);}while(0)

/*******************************************************************************
 * ������������
 * input: __vpIdx/0 - �������/��λ
 * output: no
 * return: no
 *******************************************************************************/
 #if	1
void vp_play(u8 __vpIdx)
{
	u8	bit_count = __vpIdx;
	//start
	MVOPRESET_L();
    MVOPDATA_L();
    MPLUSE_DATAWAIT("�ȴ�Լ100us");
    //reset
   	MVOPRESET_H();
    MDLUSE_RESETWAIT("�ȴ�Լ200us");
    MVOPRESET_L();
    MDLUSE_RESETWAIT("�ȴ�Լ200us");
    //reset over. begin send data
    while(bit_count > 0) {
    	MVOPDATA_H();
    	MPLUSE_DATAWAIT("�ȴ�Լ100us");
    	MVOPDATA_L();
    	MPLUSE_DATAWAIT("�ȴ�Լ100us");
    	
    	bit_count--;
    }
    if(__vpIdx > 0) {
    	IRQ_disable();
		g_tmr_iVopBusy = 0;
		IRQ_enable();
	}
    return;
}
#endif

/** ��ն��в�������ʼָ�������� **/
#if	0
void vp_HiPlay(u8 __vpIdx)
{
	actionQueueInit(&g_promptQueue, &(g_timer[2]), CPMT_TOUT, CPMT_OVER);
	SetTimer_irq(&(g_timer[2]), TIMER_10SEC, CPMT_TOUT);
	
	vp_play(__vpIdx);
}
#endif