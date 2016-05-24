
/*********************************************************************************
 * �ļ���  ��main.c
 * ����    ��         
 * ʵ��ƽ̨�����stm8������
 * ����    ��
 * ����    ��
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
 * g_flag.6 = 1.reserved (��ǰ����finger״̬.ָ��ͷ����ָ����.)
 * g_flag.7 = 1.��һ�δ���(����)���ְ���.
 * g_flag.8 = 1.reserved ADC�͵���������.(�ĳ�adcData_t�ṹ�������ô˱�־)
 * g_flag.9 = 1.�͵�ȷ��. 
 * g_flag.10 = 1. reserved
 * g_flag.11 = 1. reserved 
 * g_flag.12 = 1. ��е�����ϴ�״̬(����ǰ)
 * g_flag.13 = 1. ��е�����ϴ�״̬(������)
 * g_flag.14 = 1. reserved
 *********************************************/
u8	g_IT_flag;
/*********************************************
 * g_IT_flag 
 * g_IT_flag(3..0) reserved 
 * g_IT_flag.4 = 1. awu(awu�жϲ�����. �����ط���⵽�˱�Ǻ��������)
 * g_IT_flag.5 = 1. FP(ָ�ƴ�����)
 * g_IT_flag.6 = 1. reserved (���ܼ�����)
 * g_IT_flag.7 = 1. reserved (��������)
 *********************************************/

short g_tick;			/** ��ડ�һ����״̬ת��ʱ���� **/
short g_tmr_key;
short g_tmr_setkey;		//setKey
short g_tmr_vopkey;
iicData_t	g_iic2;

//u16 g_u16UserId = 0xffff;		/** ��ǰ�û�id��Ҳ�ǹ�����id **/
u16 g_u16AddUserId = 0xffff;	/** ��ӵ�id��(�ڶ����ܣ�ָ��+ָ�ƿ���ʱ������һ�ε�ָ��ID) **/

fstack_t g_fstack;
msgq_t  g_msgq;
Timer_t g_timer[TIMER_NUM];		/** Լ��g_timer[0]������sysProcess()�У� g_timer[1]ֻ����sysProcess()�� **/

const char g_magic[MAGIC_SIZE] = {0x48, 0x55, 0x41, 0x52}; 		/** 'H', 'U', 'A', 'R'**/

charBuf_queue_t g_com1TxQue;
//charBuf_queue_t g_com2TxQue;
charBuf_queue_t g_com3TxQue;
//charBuf_queue_t g_comRevBuf;             /** data fragment **/

/*******************************************************************************
 * �ռ�滮����:
 * g_uart1buf[0]��Ϊinָ�룬��ʼֵΪ1
 * g_uart1buf[1]��Ϊoutָ�룬��ʼֵΪ1
 * g_uart1buf[2..23+2]��Ϊ����緳�������ʼֵΪ0
 * uart1�ж���ӣ���ѭ���г���
 *******************************************************************************/
u8 g_uart1buf[CMAX_FPBUF];		//uart1�����տռ�(ָ�ƴ�������������)
/*******************************************************************************
 * �ռ�滮����:
 * g_uart1buf[0]��Ϊinָ�룬��ʼֵΪ1
 * g_uart1buf[1]��Ϊoutָ�룬��ʼֵΪ1
 * g_uart1buf[2..23+2]��Ϊ����緳�������ʼֵΪ0
 * uart1�ж���ӣ���ѭ���г���
 *******************************************************************************/
u8 g_uart3buf[CMAX_BTBUF];		//uart3�����տռ�(bluetoothʶ�����շ�����)
/*******************************************************************************
 * ��������. ÿ�β����˶���ǰ��Ҫ�����Դ˶�����գ�������ָ����ʱ��Ϣ 
 * ��ǰָ���ô˶��еĶ�����
 * 1.����(1sec�ļ���)
 * 2.ָ�ƴ�������ʱ(100ms)
 *******************************************************************************/
actionQueue_t g_actionQueue;
/** �������� **/
actionQueue_t g_promptQueue;
/** led��ʾ���� **/
actionQueue_t g_blinkQueue;

adcData_t g_adcData;

int main()
{
    int i;
	msg_t msg;
	func_t func;
	
/** �����ʼ����ʼ **/
	Peripheral_Init();
/** �����ʼ������ **/

    charQueueInit(&g_com1TxQue);
    //charQueueInit(&g_com2TxQue);
    MFPACK_FIFO_CLEAN("���ָ�ƴ�����������");
    MBTACK_FIFO_CLEAN("����������ջ�����");
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

		if(msgq_out_irq(&g_msgq, &msg) == FALSE) {     	/** ����Ϣ��? **/
			continue;
		}
		if(sysProcess(&msg) == TRUE) {   			/** ��ϵͳ��Ϣ��? **/
			continue;
		}

		if(fstack_top(&g_fstack, &func) == FALSE) {    /** ��ǰ���ڹ���״̬��? **/
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
