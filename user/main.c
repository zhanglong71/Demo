
/*********************************************************************************
 * 文件名  ：main.c
 * 描述    ：         
 * 实验平台：青风stm8开发板
 * 描述    ：
 * 作者    ：
**********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "stm8s_i2c.h"
#include "stm8s_iwdg.h"
//#include "led.h"
#include "tim1.h"
#include "exti.h"
#include "iwdg.h"
#include "uart.h"
#include "wwdg.h"
 
#include "arch.h"
#include "sys.h"
#include "charQue.h"
#include "driver.h"
#include "l_oled.h"
#include "l_sleep.h"
#include "l_i2c_touch.h"
#include "l_fingprint.h"
#include "f_idle.h"
#include "f_poweron.h"

u16 g_flag;
/*********************************************
 * g_flag 
 * g_flag.4 = 1.reserved
 * g_flag.5 = 1.reserved
 * g_flag.6 = 1.reserved (先前用于finger状态.指纹头有手指按下.)
 * g_flag.7 = 1.第一次触摸(激活)数字按键.
 * g_flag.8 = 1.reserved ADC低电检测采样完成.(改成adcData_t结构后已弃用此标志)
 * g_flag.9 = 1.低电确认. 
 * g_flag.10 = 1. reserved
 * g_flag.11 = 1. reserved 
 * g_flag.12 = 1. 机械按键上次状态(消抖前)
 * g_flag.13 = 1. 机械按键上次状态(消抖后)
 * g_flag.14 = 1. reserved
 *********************************************/
u8	g_IT_flag;
/*********************************************
 * g_IT_flag 
 * g_IT_flag(3..0) reserved 
 * g_IT_flag.4 = 1. awu(awu中断产生了. 其它地方检测到此标记后立即清除)
 * g_IT_flag.5 = 1. FP(指纹传感器)
 * g_IT_flag.6 = 1. reserved (功能键按下)
 * g_IT_flag.7 = 1. reserved (触摸按键)
 *********************************************/

short g_tick;			/** 嘀嗒。一般在状态转换时清零 **/
short g_tmr_key;
short g_tmr_setkey;		//setKey
short g_tmr_vopkey;
iicData_t	g_iic2;

//u16 g_u16UserId = 0xffff;		/** 当前用户id，也是管理者id **/
u16 g_u16AddUserId = 0xffff;	/** 添加的id号(第二功能：指纹+指纹开锁时保存上一次的指纹ID) **/

fstack_t g_fstack;
msgq_t  g_msgq;
Timer_t g_timer[TIMER_NUM];		/** 约定g_timer[0]不用在sysProcess()中， g_timer[1]只用在sysProcess()中 **/

const char g_magic[MAGIC_SIZE] = {0x48, 0x55, 0x41, 0x52}; 		/** 'H', 'U', 'A', 'R'**/

charBuf_queue_t g_com1TxQue;
//charBuf_queue_t g_com2TxQue;
charBuf_queue_t g_com3TxQue;
//charBuf_queue_t g_comRevBuf;             /** data fragment **/

/*******************************************************************************
 * 空间规划如下:
 * g_uart1buf[0]作为in指针，初始值为1
 * g_uart1buf[1]作为out指针，初始值为1
 * g_uart1buf[2..23+2]作为接收绶冲区，初始值为0
 * uart1中断入队，主循环中出队
 *******************************************************************************/
u8 g_uart1buf[CMAX_FPBUF];		//uart1最大接收空间(指纹传感器控制命令)
/*******************************************************************************
 * 空间规划如下:
 * g_uart1buf[0]作为in指针，初始值为1
 * g_uart1buf[1]作为out指针，初始值为1
 * g_uart1buf[2..23+2]作为接收绶冲区，初始值为0
 * uart1中断入队，主循环中出队
 *******************************************************************************/
u8 g_uart3buf[CMAX_BTBUF];		//uart3最大接收空间(bluetooth识别码收发缓存)
/*******************************************************************************
 * 动作队列. 每次操作此队列前，要求必须对此队列清空，并重新指定超时消息 
 * 当前指定用此队列的动作：
 * 1.开锁(1sec的级别)
 * 2.指纹传感器延时(100ms)
 *******************************************************************************/
actionQueue_t g_actionQueue;
/** 语音队列 **/
actionQueue_t g_promptQueue;
/** led显示队列 **/
actionQueue_t g_blinkQueue;

adcData_t g_adcData;

int main()
{
    int i;
	msg_t msg;
	func_t func;
	
/** 外设初始化开始 **/
	Peripheral_Init();
/** 外设初始化结束 **/

    charQueueInit(&g_com1TxQue);
    //charQueueInit(&g_com2TxQue);
    MFPACK_FIFO_CLEAN("清空指纹传感器缓冲区");
    MBTACK_FIFO_CLEAN("清空蓝牙接收缓冲区");
    actionQueueInit(&g_actionQueue, &(g_timer[1]), CACT_TOUT, CACT_OVER);
    actionQueueInit(&g_promptQueue, &(g_timer[2]), CPMT_TOUT, CPMT_OVER);
    actionQueueInit(&g_blinkQueue, &(g_timer[3]), CBLK_TOUT, CBLK_OVER);
    
    adcSample_Init(&g_adcData, &(g_timer[4]), CADC_TOUT, TIMER_300MS);
    
    for(i = 0; i < TIMER_NUM; i++) {
        ClrTimer(&g_timer[i]);
    }
    
    //SetTimer(&g_timer[0], TIMER_1SEC, CMSG_TMR);
    msgq_init(&g_msgq);
    
	#if	1
	msg.msgType = CMSG_PWON;
	msgq_in(&g_msgq, &msg);
	
	fstack_init(&g_fstack);
    func.func = f_idle;
    fstack_push(&g_fstack, &func);
	#endif
	
	AWU_Config();
	//enableInterrupts();
	IRQ_enable();
	//MIRQ_disable();
	/* Infinite loop */
	while(1)
	{
        //IWDG_ReloadCounter();
        
       	//Refresh_WWDG_Window();
       	//Test_WWDGReset();
       	
        keyscan();
        //vop_busy();
        //fingerCheck();
        PeripheralInput_Check();
        
        DAEMON_USART1_Send(&g_com1TxQue);   /** output to fingerprint **/
        DAEMON_USART3_Send(&g_com3TxQue);   /** output to bluetooth **/
        //DAEMON_USART1_Recive(&g_comRevBuf);
        
        actionDoing(&g_actionQueue);
        actionDoing(&g_promptQueue);
        actionDoing(&g_blinkQueue);

		if(msgq_out_irq(&g_msgq, &msg) == FALSE) {     	/** 有消息吗? **/
			continue;
		}
		if(sysProcess(&msg) == TRUE) {   			/** 是系统消息吗? **/
			continue;
		}

		if(fstack_top(&g_fstack, &func) == FALSE) {    /** 当前处于工作状态吗? **/
		  /** something wrong happend, Power Down or recover it **/
		  	fstack_init(&g_fstack);
			func.func = f_idle;
			fstack_push(&g_fstack, &func);
			
			g_tick = 0;
			SetTimer_irq(&g_timer[0], TIMER_1SEC, CMSG_TMR);
			continue;
		}
		func.func((unsigned *)&msg);
	}
}
