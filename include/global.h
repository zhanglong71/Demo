#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/*******************************************************************************/
extern	u16 g_flag;
extern	u8	g_IT_flag;
extern	short g_tick;
extern  int g_tmr_iVopBusy;

extern  u8 g_T_UART1Busy;
extern  u8 g_T_UART3Busy;

extern  u8 g_u8menuNO;
extern	u8 g_u8pswordSel;
extern	u8 g_u8pswordNO;
extern	u8 g_u8pswordLen;
extern	u8 g_u8password[CPSWORD_MAXLEN * 2];	//������󳤶ȿռ�

extern	u8 g_u8bluetooth[CBLUETOOTH_MAXLEN];
extern	u8 g_CMD[24];		/** only for send data fingerPrint command, bluetooth id **/

extern  u16 g_u16FPid[3];	//����+��һ��+�ڶ���

//extern	u16 g_u16UserId;		/** ��ǰ�û�id��Ҳ�ǹ�����id **/
extern	u16 g_u16AddUserId;		/** ��ӵ�id�� **/

extern	fstack_t g_fstack;
extern	msgq_t  g_msgq;
extern	Timer_t g_timer[TIMER_NUM];		/** Լ��g_timer[0]������sysProcess()�У� g_timer[1]ֻ����sysProcess()�� **/
extern  iicData_t	g_iic2;
extern  short g_tmr_key;
extern  short g_tmr_setkey;
extern  short g_tmr_vopkey;
extern	volatile u8 g_tmr_sbuf;


extern charBuf_queue_t g_com1TxQue;
extern charBuf_queue_t g_com3TxQue;
extern u8 g_uart1buf[CMAX_PSBUF];		//��󳤶�ָ������Ŀռ�
extern u8 g_uart3buf[CMAX_BTBUF];		//��󳤶��������ݵĿռ�

extern actionQueue_t g_actionQueue;
extern actionQueue_t g_promptQueue;
extern actionQueue_t g_blinkQueue;
extern adcData_t g_adcData;
/*******************************************************************************/
#endif
