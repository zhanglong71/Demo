#include "stm8s_tim1.h"
#include "tim1.h"

#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "arch.h"
//#include "led.h"

static  u32 TimingDelay; 

void Tim1_Init(void)
{
  TIM1_TimeBaseInit(16,TIM1_COUNTERMODE_UP,1000,0);//16分频，向上计数，计数完了触发中断，如果想
//再 1MS 触发中断，则计数器应该计数 1000 次，1MHZ/1000=1KHZ，正好就是 1ms。
  TIM1_ARRPreloadConfig(ENABLE);//使能自动重装
  TIM1_ITConfig(TIM1_IT_UPDATE , ENABLE);//数据更新中断
  TIM1_Cmd(ENABLE);//开定时器
}

#define	TMR_COM1IDLE (TIMER_10MS)
#define	TMR_COM3IDLE (TIMER_10MS)
//u8 g_T_UART1Busy = TMR_COMIDLE;
u8 g_T_UART1Busy = TMR_COM1IDLE;      /** 边界 **/
u8 g_T_UART3Busy = TMR_COM3IDLE;      /** 边界 **/

int g_tmr_iVopBusy;
/*******************************************************************************
 * 
 *---CVOPTMR_RESET(3)-------CVOPTMR_S(10)-----------------------------CVOPTMR_INIT---->
 *
 *******************************************************************************/
void TimingDelay_Decrement(void)
{
    int i;
	msg_t msg;
	if (TimingDelay != 0x00){
		TimingDelay--;
	}
	/***************************************************/
    for(i = 0; i < TIMER_NUM; i++) {
        if(g_timer[i].tick_bak > 0) {
            if(g_timer[i].tick > 0) {
                g_timer[i].tick--;
            } else {
                g_timer[i].tick = g_timer[i].tick_bak;
                
                msg.msgType = g_timer[i].msgType;		/** message type **/
                msg.msgValue = i;						/** message value(Timer index) **/
				msgq_in(&g_msgq, &msg);
            }
        }
    }
    /***************************************************/
    if(g_tmr_key > TIMER_KEYCHK /** 触摸动作确认时间点 **/) {
    	g_tmr_key--;
    } else if(g_tmr_key > 0) {
    	g_tmr_key = 0;
    } /** else if (g_tmr_key == 0){} **/
    /***************************************************/
    if(g_tmr_setkey > TIMER_KEYCHK /** set按键确认消除抖动 **/) {
    	g_tmr_setkey--;
    }
    /***************************************************/
    if(g_tmr_vopkey > TIMER_KEYCHK /** 语音按键确认时间点 **/) {
    	g_tmr_vopkey--;
    } else if (g_tmr_vopkey > 0) { /** (0, TIMER_KEYCHK] **/
    	g_tmr_vopkey = 0;
    	
		msg.msgValue = CKEY_VOP;
		msg.msgType = CMSG_DKEY;
		msgq_in(&g_msgq, &msg);
    } /** else if (g_tmr_vopkey == 0){} **/
    /***************************************************/
    g_T_UART1Busy++;
    if(g_T_UART1Busy > TMR_COM1IDLE) {	 /** out of 60ms **/
    	g_T_UART1Busy = TMR_COM1IDLE;
    } else if(g_T_UART1Busy == TMR_COM1IDLE) {	/** time out **/
		msg.msgValue = 1;
		msg.msgType = CMSG_COMRX;
		msgq_in(&g_msgq, &msg);	  	/** 收到一组完整数据. 发消息 **/
    } /**  else {} **/ 
    /***************************************************/
    g_T_UART3Busy++;	/** for bluetooth **/
    if(g_T_UART3Busy > TMR_COM3IDLE) {	 /** out of 60ms **/
    	g_T_UART3Busy = TMR_COM3IDLE;
    } else if(g_T_UART3Busy == TMR_COM3IDLE) {	/** time out **/
		msg.msgValue = 1;
		msg.msgType = CMSG_UART3RX;
		msgq_in(&g_msgq, &msg);	  	/** 收到一组完整数据. 发消息 **/
    } /**  else {} **/ 
    /***************************************************/
  #if 1
    if(g_tmr_iVopBusy < TIMER_VOPBUSY) {
      g_tmr_iVopBusy++;
    }
  #endif
  	/***************************************************/
}

void delay_ms( u32 nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}
