#ifndef __PTYPE_H__
#define __PTYPE_H__

/*******************************************************************************
 *
 * All data type define here
 *
 *******************************************************************************/
 
/*******************************************************************************/
typedef enum {
    FAILED = 0, 
    PASSED = !FAILED
} RetStatus;

typedef	int		(*pfunc_t)(unsigned *arg);
typedef	void	(*paction_t_0)(void);
//typedef void 	(*pfunc_v_u8_u8_u8_t)(u8 x, u8 y, u8 ch);
typedef void 	(*pfunc_v_u8_u8_u32_u8_u8_t)(u8 x, u8 y, u32 num, u8 len, u8 size2);

typedef struct func_s {
    pfunc_t		func;       /** function **/
    unsigned int  *arg;    /** argument **/
} func_t;

typedef struct func_stack{
	//int		top;
	u8		top;
	func_t		func[STACKSIZE]; 
}fstack_t;

typedef enum {
    CMSG_NONE = 0, 
/*************
 * �����ⶨ��Ϣ��������
 * 1. ��ͨ��CMSG_xxx��ʽ����
 * 2. ��CSYS_xxx����ʽ������sys�д������Ϣ
 *************/    
    CMSG_PWON,  	//power on(��Ҫ��ʼ�����������)
    CMSG_RESU,  	//resume (�ӹ���״̬�ָ�����Ҫ��ʼ����������ݼ�����)
    CMSG_PEND,  	//pending (��������ֹͣ����)
    
    CMSG_INIT,  	//��ʼ������(����ѡ��)
    CMSG_SECO,  	//��ʼ������(�ٴν���ָ�Ʊȶ�״̬)
    CMSG_BACK,  	//��ʼ������(����ѡ��)
    CINIT_ENROLL, 	//��ʼ������ - ע�����
    CINIT_MATCH,  	//��ʼ������ - ƥ��
    //CMSG_BACK,  	//��ʼ������(�˳�ѡ��)
    
    CMSG_TMR,
    CMSG_DLAY, 	/** ʱ�� **/
    CMSG_NEXT, 	/** ��������һ�׶� **/
    CMSG_DKEY,
    CMSG_UKEY,
    
    //CMSG_FGON,	/** ��⵽ָ��ͷ������ָ **/
    //�Ѿ�׼���õ�
    CFIGER_ON,
    CFIGER_INIT,	/** ׼���ã���λָ��ͷ **/
    CFIGER_ADD,		/** ׼���ã�ע��ָ�� **/
    CFIGER_DEL,		/** ׼���ã�ɾ��ָ�� **/
    CFIGER_TOUT,	/** ָ��ͷ��Ӧ��ʱ **/
    CFIGER_FGPRPRD,	/** ���׼�� **/
    
    CMSG_FGINIT,		/** ��ʼ���ɹ� **/
    CMSG_FGRGST,		/** ע��ָ�Ƴɹ� **/
    CMSG_FGRGSTING,		/** ע��ָ�ƶ������� **/
    CMSG_FGQUERY,		/** ����ָ�Ƴɹ� **/
    CMSG_FGQUERYING,	/** ����ָ�ƶ�������  **/
    CMSG_FGDERG,		/** ע��ָ�Ƴɹ� **/
    CMSG_FGDERGING,		/** ע��ָ�ƶ������� **/
    CMSG_FGDELALL,		/** ע��ȫ��ָ�Ƴɹ� **/
    CMSG_FGDELALLING,	/** ע��ȫ��ָ�Ƽ��� **/
    CMSG_FGOPFAIL,		/** ָ��ͷ����ʧ��() **/
    CMSG_FGCOMFAIL,		/** ͨ��ʧ��(��Ч����) **/
    
    CMSG_FGTMPEMPTY,	/** ָ��ģ����� **/
    CMSG_FGTMPBUSY,		/** ָ��ģ�岻���� **/
    
    CMSG_FGINTEGRITY,	/** ָ��ģ������ **/
    CMSG_FGNOTINTEGR,	/** ָ��ģ�岻���� **/
    
    CMSG_BLELINK,		/** ��������������(���Է�������) **/    
    //CMSG_FGCCEL,		/** �˳������ɹ�(0x0130�ش�) **/
    //CMSG_FGCCELING,	/** �˳������ɹ�(0x0130�ش�) **/
    
    CMSG_FGDOING,		/** ָ�ƴ������ **/
    
	CNOKEY_ENTER,		/** ��Կ�׽��� **/
	
    CMSG_COMTX,
    CMSG_COMRX,
    
    CMSG_UART3RX,
    
    CMSG_RMT,			/** remote **/
    CMSG_PRY,			/** anti-pry **/
    
    //CMSG_BLTOOTH,		/** ����������Ϣ **/
	CMSG_BTMATCH,		/** �������ݲ�ѯƥ�� **/
    CMSG_BTENROLL,		/** ��������ע�� **/
    
	/**  **/
	//CVOP_STRT,		/** voice prompt start **/
	//CVOP_OVER,

	CADC_TOUT,		/** ������ʱ **/
	CADC_OVER,		/** ������� **/
	
	CACT_TOUT,		/** ������ʱ **/
	CACT_OVER,		/** ������� **/
	
	CPMT_TOUT,		/** ��ʾ������ʱ **/
	CPMT_OVER,		/** ��ʾ������� **/
	
	CBLK_TOUT,		/** blink��ʾ������ʱ **/
	CBLK_OVER,		/** blink��ʾ������� **/

    //CMSG_ADC,		/** ADת�� **/
    //CMSG_CALC,		/** ����У��͵���յ� **/
    
    CUART1_RCV,
    CUART2_RCV,
    
    CIIC_INIT,		/** IIC�ӿڳ�ʼ�� **/
    
    CMSG_COMTIMEOUT,
} msgType_t;

typedef struct msg_s {
	msgType_t msgType;
	//int	msgValue;
	u8	msgValue;
}msg_t;

typedef struct msg_queue{
	u8	head;
	u8	tail;
	msg_t msg[QUEUESIZE];
} msgq_t;

typedef struct Timer_s {
    u16 tick_bak;
    u16 tick;
    msgType_t msgType;                      /** the type of msg when timeout **/
    //unsigned int count;                   /** Is it necessary ? **/
}Timer_t; 


typedef struct bitmap_s {
    //unsigned int bitmap;		    /** 32 bits **/
    u8 bitmap[40];				/** 40 * 8 = 320bits **/
} bitmap_t;

typedef enum {              /** ���ڿ���com���ݵķ��� **/
    CTRL_START = 0,         /** 0 - start transmission **/
    CTRL_CONTI = 1,         /** 1 - continue transmission  **/
                            /** 2 reserved. for other control **/
    CTRL_TMR = 5,           /** 3..200 for delay **/
    CTRL_STOP = 255,        /** 255 - stop transmission**/
} ctrlFlag_t;

typedef struct charData_s{
	u8 ucVal;
	//ctrlFlag_t ctrl;         /** �����ֶΣ����ڱ�ﵱǰ��data����Ч���ݻ����ӳٻ��������� **/
} charData_t;

typedef struct charBuf_queue_s {
    //int	head;
	//int	tail;
    //int len;
    u8	head;
	u8	tail;
    u8 len;
    //#define CHARQUEUESIZE   16
    #define CHARQUEUESIZE   32
    u8 buf[CHARQUEUESIZE];     /** 16 bytes **/
} charBuf_queue_t;


typedef struct action_s {
	/*****************
	 * MOT-ON
	 * MOT-OFF
	 * LED-ON
	 * LED-OFF
	 * VOP-ON
	 * VOP-OFF
	 * Delay
	 *****************/
#define CACT_MOTONORM	0xff
#define CACT_MOTOREVE	0xfe
#define CACT_MOTOWAIT	0xfd
  
#define CACT_LEDPMTON	0xfb
#define CACT_LEDPMTOFF	0xfa
#define CACT_LEDPMTWAT	0xf9
  
#define CACT_LEDWRNON	0xf7
#define CACT_LEDWRNOFF	0xf6
#define CACT_LEDWRNWAT	0xf5
  
#define CACT_VOPON	0xf3
//#define CACT_VOPOFF	0xf2
#define CACT_VOPWAT	0xf1
//#define CACT_DELAY	0xf0
  
#define CACT_BEEPON	0xef
#define CACT_BEEPOFF	0xee


#define CACT_MISC	0xeb


    paction_t_0		func;
    //unsigned int actionTime;	/** ����ʱ�� **/
    //unsigned int actionPara;	/** ������Ҫ��������������VPidx������ **/
    u16 actionTime;	/** ����ʱ�� **/
    u16 actionPara;	/** ������Ҫ��������������VPidx������ **/
    u8 actionType;	/** ��������. ����ǰ��Ĳ������ص�֮�ӣ���һ�������������� **/
    u8 __pad1;		/** �ֽڶ��� **/
} action_t;

typedef struct actionQueue_s {
    //int	head;
	//int	tail;
    //int flag;
    
    u8	head;
	u8	tail;
    u8 flag;
	msgType_t stepMsgType;			/** �׶��¼������Ϣ **/
	msgType_t overMsgType;			/** �����¼�ȫ�������Ϣ **/
	Timer_t *timer;
        #define ACTIONQUEUESIZE 16
    action_t buf[ACTIONQUEUESIZE];     /** 16 bytes **/
} actionQueue_t;
/*******************************************************************************/
typedef struct iicTxData_s{
	u8 addr;
	u8 data;
} iicTxData_t;

typedef struct iicData_s{
	u8 addr;
	u8 data;
	u8 stat;		//���ڱ�ʶ����״̬
	/************************************************** 
		stat = 	0 �¼��ش���������(�ֶ�����ʱ)
				1 addr���ڷ�����(���Ͷ���ʱ)
				2 data���ڷ�����(���Ͷ���ʱ)
				3 ���data����(����data��ɣ���ֹͣ)	
	 **************************************************/
	u8 _pad;		//reserved
} iicData_t;
/*******************************************************************************/
typedef struct adcData_s{
    u16	u16count;				//����
	u16	u16currValue;			//��ǰֵ
    u16 u16average;				//��ֵ
    
	u16 tick;
	msgType_t stepMsgType;		/** �׶��¼������Ϣ **/
	Timer_t *timer;
} adcData_t;
/*******************************************************************************/
#if	0
typedef union {
	iicData_t	iicData;
	u8	array[sizeof(iicData_t)];
} iicData_u;
#endif
/*******************************************************************************/
#if 0
typedef union flashPage_u {
	u8 arrChar[CFLASH_PAGE_SIZE];
	u32  arrInt[CFLASH_PAGE_SIZE/4];
} flashPage_t;
#endif
/*******************************************************************************/

#endif /** ifndef end **/
