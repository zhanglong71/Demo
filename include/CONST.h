#ifndef __CONST_H__
#define __CONST_H__

/*******************************************************************************/

/*******************************************************************************
 *
 * constant define here
 *
 *******************************************************************************/
#define	STACKSIZE	8
#define	QUEUESIZE	8

//#define	OK	0
//#define	ERROR	-1

#define	TRUE	1
#define	FALSE	0

#define CSEND_BUF_NUM	8	//16	 //����������
#define CSEND_BUF_SIZE	16	//16	 //��������С

#define TIMER_NUM	5       /** ��ʱ������ **/
/** 
 * 0 - ��ʱ����
 * 1 - ��/����
 * 2 - VOP����
 * 3 - led blink
 * 4 - ADC�ɼ�����
 */
#define TIMER_0 	0		/** 1kHz��ʱ, 0���ڹر�ʱ�� **/
#define TIMER_10MS (10)		/** 1kHz��ʱ, 10ms�������� **/

#define TIMER_20MS	(TIMER_10MS * 2)		/** 1kHz��ʱ, 20ms�������� **/
#define TIMER_30MS	(TIMER_10MS * 3)		/** 1kHz��ʱ, 20ms�������� **/
#define TIMER_50MS	(TIMER_10MS * 5)		/** 1kHz��ʱ, 50ms�������� **/
#define TIMER_60MS 	(TIMER_10MS * 6)		/** 1kHz��ʱ, 60ms�������� **/
#define TIMER_80MS	(TIMER_10MS * 8)		/** 1kHz��ʱ, 80ms�������� **/
#define TIMER_100MS (TIMER_10MS * 10)		/** 1kHz��ʱ, 100ms�������� **/

#define TIMER_200MS (TIMER_100MS * 2)		/** 1kHz��ʱ, 200ms�������� **/
#define TIMER_300MS (TIMER_100MS * 3)		/** 1kHz��ʱ, 300ms�������� **/
#define TIMER_500MS (TIMER_100MS * 5)		/** 1kHz��ʱ, 500ms�������� **/
#define TIMER_600MS (TIMER_100MS * 6)		/** 1kHz��ʱ, 600ms�������� **/
#define TIMER_700MS (TIMER_100MS * 7)		/** 1kHz��ʱ, 700ms�������� **/
#define TIMER_800MS (TIMER_100MS * 8)		/** 1kHz��ʱ, 800ms�������� **/
#define TIMER_1SEC  (TIMER_100MS * 10)		/** 1kHz��ʱ, ÿ1��������� **/

#define TIMER_2SEC  (TIMER_1SEC * 2)	/** 1kHz��ʱ, ÿ2��������� **/
#define TIMER_3SEC  (TIMER_1SEC * 3)	/** 1kHz��ʱ, ÿ3��������� **/
#define TIMER_4SEC  (TIMER_1SEC * 4)	/** 1kHz��ʱ, ÿ4��������� **/
#define TIMER_5SEC  (TIMER_1SEC * 5)	/** 1kHz��ʱ, ÿ5��������� **/
#define TIMER_8SEC  (TIMER_1SEC * 8)	/** 1kHz��ʱ, ÿ5��������� **/
#define TIMER_10SEC (TIMER_1SEC * 10)	/** 1kHz��ʱ, 10��������� **/
/**  **/
//#define TIMER_CALIB 800000	/** 1kHz��ʱ, 100�볬ʱ�˳� **/
//#define TIMER_RGST  800		/** ע��ʱ���� **/
#define TIMER_KEYCHK  (TIMER_10MS)

#define TIMER_VOPBUSY  60								/** ����æ�����ʱʱ�� **/
#define TIMER_VOPBUSYMAX  (TIMER_VOPBUSY + 30)							/** �������������ʱʱ���������� **/
//#define TIMER_VOPBUSYFIXED  ((TIMER_VOPBUSYMAX) + 10)	/** �������������ʱʱ��̶�ֵ **/

#define TIMER_VOPFAULTBEGIN  10							/** ������������쳣ʱ���������� **/
#define TIMER_VOPFAULTEND  (TIMER_VOPFAULTBEGIN + 8000) /** ������������쳣ʱ���������� **/

#define CTIMER_CALIB (2000)	/** ����У��״̬��Ҫ�������°�����ʱ��, ն��Ϊ2s **/

/***************** ���� **************************/
#define	CKEY_1	(0x01)
#define	CKEY_2	(0x02)
#define	CKEY_3	(0x03)
#define	CKEY_4	(0x04)
#define	CKEY_5	(0x05)
#define	CKEY_6	(0x06)
#define	CKEY_7	(0x07)
#define	CKEY_8	(0x08)
#define	CKEY_9	(0x09)
#define	CKEY_asterisk	(0x0A)
#define	CKEY_0	(0x0b)
#define	CKEY_pound	(0x0C)

#define	CKEY_NONE	(0x00)
#define	CKEY_WU	(0x0d)

#define	CKEY_X3	(0x0e)
#define	CKEY_X4	(0x0f)

#define	CKEY_GOOUT	(0x10)
#define	CKEY_GOIN	(0x11)
#define CKEY_SET 	(0x12)		//�ź�����ü�
#define CKEY_VOP 	(0x13)		//�ź��������
/*************************************************/
#define CPSWORD_MAXLEN	(30 + 2)
#define CPSWORD_MINLEN	(4)
#define CPSWORD_MAXCNT	(11)
#define CPSWORDLEN	(CPSWORD_MAXLEN - 10)
#define	CPSWORDCNT	(6 + 1)		/** ��0�����뱣�� **/
/*************************************************/
#define	CRECORD_MAXLEN	(16)
#define	CRECORD_MAXCNT	(1024)
/*******************************************************************************/
#define CBLUETOOTH_MAXLEN	(30 + 2)		//����ռ�(�������ֵ)
#define CBLUETOOTH_MINLEN	(4)
#define CBLUETOOTH_MAXCNT	(11)			//����ռ�(�������ֵ)
#define CBLUETOOTHLEN	(CBLUETOOTH_MAXLEN - 10)	//ʵ��
#define	CBLUETOOTHCNT	(6 + 1)		/** ��0��λ�ñ��� **/

#define	AT_CMD_MAXLEN	(32)	//AT����ĳ���


#define	CCMD_BTHEAD	(0x5a)
#define	CCMD_ENROLL	(0x5a)
#define	CCMD_MATCH	(0xa5)
/*******************************************************************************/
//#define	CTOTALADDRESS	1024


/**************************** ָ�ƴ������������ *****************************/
//for JP100-A
#if	1
	#define FP_INTERRUPT			0
	#define FP_INQUIRY				1
	#define FP_FPCHECK				2
	#define FP_MATCHONE     		0x0101
	#define FP_MATCH     			0x0102	/** 1:N **/
	#define FP_ENROLL     			0x0103	/** enroll by 3 time **/
	#define FP_ENROLLONETIME    	0x0104	/** enroll by one time **/
	#define FP_DELETE    			0x0105	/** delete one **/
	#define FP_DELALL    			0x0106	/** delete all **/
	//#define FP_GETEMPTY    			0x0107	/** get empty id **/
	#define FP_TMPSTATE    			0x0108	/** get the state with specified template **/
	#define FP_TMPINTEG    			0x0109	/** get the integrity with specified template **/
	#define FP_CANCEL    			0x0130	/** cancel operation **/
	
	
	#define FP_INCORRECT_COMMAND    0x0160	/** incorrect command **/
#else
//for 3001D
	#define FP_INTERRUPT			0
	#define FP_INQUIRY				1
	#define FP_FPCHECK				2
	#define FP_IMAGE				2
	#define FP_ENROLL     			3
	#define FP_MATCH     			4
	#define FP_DELETE    			5
#endif


/** ������ݴ����Ƿ����� **/
#define	CMD_ACK_OK (0)		//�������޴���
#define	CMD_ACK_SUM_ERR (2)
//#define	CMD_EROLL_AGAIN (0x8301)	//��ע��(�ظ�ע��)
//#define	CMD_ACK_XOR_ERR (1)

/*******************************************************************************
 * ָ�ƴ����� 
 *******************************************************************************/
#define	CRESP_ERR_SUCCESS 			(0)
#define	CRESP_ERR_FAIL 				(1)
/** ƥ���� **/
#define	CRESP_ERR_IDENTIFY 				(0x12)		/** 1:N�ȶ�ʧ�� **/
#define	CRESP_ERR_ALL_TMPL_EMPTY 		(0x15)		/** �������ѵǼ�ģ�� **/
#define	CRESP_ERR_BAD_QUALITY 			(0x21)		/** ͼ���������� **/
#define	CRESP_ERR_BAD_TIMEOUT 			(0x23)		/** ��ʱ **/
#define	CRESP_GD_NEED_RELEASE_FINGER	(0xFFF4)	/** �뿪��ָ **/
/** ¼���� **/

// 1. 0x00 - ָ���ɹ�
// 2. 0x01 - ָ���ʧ��
#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//ָ����Template������Ч
#define CRESP_ERR_TMPL_NOT_EMPTY	(0x14)		//ָ���������Ѵ���Template
#define	CRESP_GD_NEED_FIRST_SWEEP	(0xFFF1)	/** ��һ��ָ������ȴ� **/
#define	CRESP_GD_NEED_SECOND_SWEEP	(0xFFF2)	/** �ڶ���ָ������ȴ� **/
#define	CRESP_GD_NEED_THIRD_SWEEP	(0xFFF3)	/** ������ָ������ȴ� **/
//#define	CRESP_ERR_BAD_TIMEOUT 			(0x23)	//Timeoutʱ����û�м�⵽ָ�Ƶ�����
//#define	CRESP_ERR_BAD_QUALITY 			(0x21)	/** ָ��ͼ���������� **/
#define	CRESP_ERR_GENERALIZE 		(0x30)		/** �Ǽǵ�Template������ʧ�� **/

/** ɾ����� **/
#define CRESP_ERR_TMPL_EMPTY		(0x13)		//ָ���ĺ����в�����ָ��ģ������
#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//ָ����Template������Ч

/** ��ѯ״̬ **/
#define CRESP_GD_TEMPLATE_NOT_EMPTY	(0x01)		//ָ���ĺ����в�����ָ��ģ������
#define CRESP_GD_TEMPLATE_EMPTY		(0x00)		//ָ����Template������Ч

/** ��ѯ������ **/
//#define CRESP_ERR_TMPL_EMPTY		(0x13)		//ָ���ĺ����в�����ָ��ģ������
//#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//ָ����Template������Ч
/***************************************************************/
/****/
#define	CINVALIDID	0xffff		/** ��Чid�� **/
/*******************************************************************************/
#define	CFINGER_MAXCNT	512

//#define	CTOTALFINGER	290


/** ���е�x��ָ�ƴ������϶�Ӧ��id��(��byteͼһ��) **/
#define	MUSER_ID_BASE	(9)
#define	MUSER_ID_MAX	(290)	/** ָ��������Ŀ��, Ҳ��ָ��ģ������ **/
//#define	MUSER_ID_MAX	(CTOTALFINGER)	/** ָ��������Ŀ��, Ҳ��ָ��ģ������ **/

#define	MisSUPER(x) ((x) == 0)
#define	MisADMIN(x) ((x) < (MUSER_ID_BASE))
#define	MisUSER(x) 	(((x) >= (MUSER_ID_BASE)) && ((x) < (MUSER_ID_MAX)))
/** ָ��idת�����û�id **/
#define	M2UserID(x) ((x) - (MUSER_ID_BASE))	// x ==> UserID
#define	M2AdminID(x) (x)				// x ==> AdminID

//#define CADC1_DR_ADDRESS    ((u32)0x4001244C)

//#define	CFLASHSTARTADDR (0x00027C00)		/** the 256th Flash Page(512byte/page) **/
#define	CFLASHSTARTADDR (0x00018000)	/** the last 32kB(96KB-128KB) **/

#define CMAX_PSERR 6		//�������ʧ�ܴ���

#define CMAX_FPBUF 32		//���ָ������峤��
#define CMAX_PSBUF 32		//������뻺�峤��
#define CMAX_BTBUF 32		//����������ݻ��峤��
/*******************************************************************************/
//#define	CFLASH_PAGE_SIZE		256			//1024 �ƺ��ò�����

//#define	CFLASH_ADC_VALUE		4			/** 4..5 **/
//#define	CFLASH_ADC_FLAG			6
//#define	CFLASH_RESET_VALUE		8
//#define	CFLASH_INDUCTION_POWER		12		/** ָ��ͷ��Ӧ�ϵ��־A5/A0��Ϊ�л��ޣ�����Ϊ��ЧҲ�����и�Ӧ�ϵ��־ **/
//		#define CINDUCTION_POWER_DISABLE	0xA0		/** ��ָ���ϵ繦��(Ĭ����) **/
//		#define CINDUCTION_POWER_ENABLE		0xA5		/** ��ָ���ϵ繦��(Ĭ����) **/
#define	CFLASH_PASSWD_ERR		4		/** �������������� **/
//#define	CFLASH_PASSWD_START		32
//#define	CFLASH_FPIndex_START	320

#define	CENTRYFLAG_BUSY	0xa0	/** ��ʾָ�������ѱ�ʹ�á�ʵ��Ӧ����ֻҪ��0xF0�������0xA0���� **/
#define	CENTRYFLAG_IDLE	0x0		/** ��ʾָ������δ��ʹ��, ʵ��Ӧ����ֻҪ��0xF0���벻��0xA0���� **/

#define	MAGIC_SIZE 4
/*******************************************************************************/
//������������
#define	CVOPID_RESET	0

#define	CVOPID_SILENT	1
#define	CVOPID_BASE	1

#define	CVOPID_BEEP		(CVOPID_BASE + 1)
#define	CVOPID_Hao		(CVOPID_BASE + 2)	//��
#define	CVOPID_Xiao		(CVOPID_BASE + 3)	//Ч(��ǰû�õ�)
#define	CVOPID_Wu		(CVOPID_BASE + 4)	//��(��ǰû�õ�)
#define	CVOPID_asterisk	(CVOPID_BASE + 5)	//*
#define	CVOPID_pound	(CVOPID_BASE + 6)	//#
#define	CVOPID_Key		(CVOPID_BASE + 7)	//��
#define	CVOPID_Had		(CVOPID_BASE + 8)	//��(��ǰû�õ�)
#define	CVOPID_Full		(CVOPID_BASE + 9)	//��
#define	CVOPID_Press	(CVOPID_BASE + 10)	//��
#define	CVOPID_Please	(CVOPID_BASE + 11)	//��
#define	CVOPID_AGAIN	(CVOPID_BASE + 12)	//��
#define	CVOPID_Ci		(CVOPID_BASE + 13)	//��(��ǰû�õ�)
#define	CVOPID_PSWORD	(CVOPID_BASE + 14)	//����
#define	CVOPID_PASSED	(CVOPID_BASE + 15)	//ͨ��
#define	CVOPID_INPUT	(CVOPID_BASE + 16)	//����
#define	CVOPID_FINGER	(CVOPID_BASE + 17)	//��ָ
#define	CVOPID_ADD		(CVOPID_BASE + 18)	//���
#define	CVOPID_DEL		(CVOPID_BASE + 19)	//ɾ��
#define	CVOPID_SET		(CVOPID_BASE + 20)	//����(��ǰû�õ�)
#define	CVOPID_SYSTEM	(CVOPID_BASE + 21)	//ϵͳ(��ǰû�õ�)
#define	CVOPID_UNLOCK	(CVOPID_BASE + 22)	//����(��ǰû�õ�)
#define	CVOPID_COMPARISON	(CVOPID_BASE + 23)	//��֤(�ȶ�)
#define	CVOPID_SUCESS		(CVOPID_BASE + 24)	//�ɹ�
#define	CVOPID_FAILED		(CVOPID_BASE + 25)	//ʧ��
#define	CVOPID_FPRINT		(CVOPID_BASE + 26)	//ָ��
#define	CVOPID_LOGIN		(CVOPID_BASE + 27)	//��¼
#define	CVOPID_Doing		(CVOPID_BASE + 28)	//����...	
#define	CVOPID_Wait			(CVOPID_BASE + 29)	//�Ժ�
#define	CVOPID_CLEARED		(CVOPID_BASE + 30)	//���
#define	CVOPID_INPUT2		(CVOPID_BASE + 31)	//2
#define	CVOPID_FUNCTION		(CVOPID_BASE + 32)	//����
#define	CVOPID_CONFIRM		(CVOPID_BASE + 33)	//ȷ��
#define	CVOPID_CANCEL		(CVOPID_BASE + 34)	//ȡ��
#define	CVOPID_USER			(CVOPID_BASE + 35)	//�û�
#define	CVOPID_ALL			(CVOPID_BASE + 36)	//����
#define	CVOPID_Doing2		(CVOPID_BASE + 37)	//8
#define	CVOPID_VOICE		(CVOPID_BASE + 38)	//����
#define	CVOPID_ON		(CVOPID_BASE + 39)	//����
#define	CVOPID_OFF		(CVOPID_BASE + 40)	//�ر�
#define	CVOPID_ALARM		(CVOPID_BASE + 41)	//������.......
#define	CVOPID_ID		(CVOPID_BASE + 42)	//���
#define	CVOPID_LOCK		(CVOPID_BASE + 43)	//����
#define	CVOPID_OVER		(CVOPID_BASE + 44)	//����
#define	CVOPID_RETURN		(CVOPID_BASE + 45)	//����
#define	CVOPID_PAGEUP		(CVOPID_BASE + 46)	//�Ϸ�
#define	CVOPID_PAGEDN		(CVOPID_BASE + 47)	//�·�
#define	CVOPID_PLEASEPRESS	(CVOPID_BASE + 48)	//�밴
#define	CVOPID_INITIAL		(CVOPID_BASE + 49)	//��ʼ��
#define	CVOPID_DIFFERENT	(CVOPID_BASE + 50)	//��һ��
#define	CVOPID_MANAGER		(CVOPID_BASE + 51)	//����Ա
#define	CVOPID_ANOTHER		(CVOPID_BASE + 52)	//��һö
#define	CVOPID_CONTINUE		(CVOPID_BASE + 53)	//��������
#define	CVOPID_ALLDATA		(CVOPID_BASE + 54)	//ȫ������
#define	CVOPID_RESTORE		(CVOPID_BASE + 55)	//�ָ�����
#define	CVOPID_FORYOURSAFTY		(CVOPID_BASE + 56)	//Ϊ����İ�ȫ
#define	CVOPID_TURNHANDLE		(CVOPID_BASE + 57)	//ת�����ֿ���
#define	CVOPID_POWERLOW		(CVOPID_BASE + 58)	//�����ͣ���������
#define	CVOPID_WELCOME		(CVOPID_BASE + 59)	//��ӭʹ�û�԰ָ����

#define	CVOPID_FINAL		(CVOPID_WELCOME)	//��β


#define	CVOP_VOL0		0xe0
//...
#define	CVOP_VOL7		0xe7
#define	CVOP_LOOP		0xf2	//play voice again and again
#define	CVOP_STOP		0x00	//stop playing
/*******************************************************************************/

/*******************************************************************************/
#endif
