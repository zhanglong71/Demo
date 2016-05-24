
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
 * 启动语音播放
 * input: __vpIdx/0 - 语音序号/复位
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
    MPLUSE_DATAWAIT("等待约100us");
    //reset
   	MVOPRESET_H();
    MDLUSE_RESETWAIT("等待约200us");
    MVOPRESET_L();
    MDLUSE_RESETWAIT("等待约200us");
    //reset over. begin send data
    while(bit_count > 0) {
    	MVOPDATA_H();
    	MPLUSE_DATAWAIT("等待约100us");
    	MVOPDATA_L();
    	MPLUSE_DATAWAIT("等待约100us");
    	
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

/** 清空队列并立即开始指定的语音 **/
#if	0
void vp_HiPlay(u8 __vpIdx)
{
	actionQueueInit(&g_promptQueue, &(g_timer[2]), CPMT_TOUT, CPMT_OVER);
	SetTimer_irq(&(g_timer[2]), TIMER_10SEC, CPMT_TOUT);
	
	vp_play(__vpIdx);
}
#endif