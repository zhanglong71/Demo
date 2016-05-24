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

#define CSEND_BUF_NUM	8	//16	 //缓冲区个数
#define CSEND_BUF_SIZE	16	//16	 //缓冲区大小

#define TIMER_NUM	5       /** 定时器个数 **/
/** 
 * 0 - 计时计数
 * 1 - 开/闭锁
 * 2 - VOP语音
 * 3 - led blink
 * 4 - ADC采集周期
 */
#define TIMER_0 	0		/** 1kHz定时, 0等于关闭时钟 **/
#define TIMER_10MS (10)		/** 1kHz定时, 10ms计数次数 **/

#define TIMER_20MS	(TIMER_10MS * 2)		/** 1kHz定时, 20ms计数次数 **/
#define TIMER_30MS	(TIMER_10MS * 3)		/** 1kHz定时, 20ms计数次数 **/
#define TIMER_50MS	(TIMER_10MS * 5)		/** 1kHz定时, 50ms计数次数 **/
#define TIMER_60MS 	(TIMER_10MS * 6)		/** 1kHz定时, 60ms计数次数 **/
#define TIMER_80MS	(TIMER_10MS * 8)		/** 1kHz定时, 80ms计数次数 **/
#define TIMER_100MS (TIMER_10MS * 10)		/** 1kHz定时, 100ms计数次数 **/

#define TIMER_200MS (TIMER_100MS * 2)		/** 1kHz定时, 200ms计数次数 **/
#define TIMER_300MS (TIMER_100MS * 3)		/** 1kHz定时, 300ms计数次数 **/
#define TIMER_500MS (TIMER_100MS * 5)		/** 1kHz定时, 500ms计数次数 **/
#define TIMER_600MS (TIMER_100MS * 6)		/** 1kHz定时, 600ms计数次数 **/
#define TIMER_700MS (TIMER_100MS * 7)		/** 1kHz定时, 700ms计数次数 **/
#define TIMER_800MS (TIMER_100MS * 8)		/** 1kHz定时, 800ms计数次数 **/
#define TIMER_1SEC  (TIMER_100MS * 10)		/** 1kHz定时, 每1秒计数次数 **/

#define TIMER_2SEC  (TIMER_1SEC * 2)	/** 1kHz定时, 每2秒计数次数 **/
#define TIMER_3SEC  (TIMER_1SEC * 3)	/** 1kHz定时, 每3秒计数次数 **/
#define TIMER_4SEC  (TIMER_1SEC * 4)	/** 1kHz定时, 每4秒计数次数 **/
#define TIMER_5SEC  (TIMER_1SEC * 5)	/** 1kHz定时, 每5秒计数次数 **/
#define TIMER_8SEC  (TIMER_1SEC * 8)	/** 1kHz定时, 每5秒计数次数 **/
#define TIMER_10SEC (TIMER_1SEC * 10)	/** 1kHz定时, 10秒计数次数 **/
/**  **/
//#define TIMER_CALIB 800000	/** 1kHz定时, 100秒超时退出 **/
//#define TIMER_RGST  800		/** 注册时间间隔 **/
#define TIMER_KEYCHK  (TIMER_10MS)

#define TIMER_VOPBUSY  60								/** 语音忙检测延时时间 **/
#define TIMER_VOPBUSYMAX  (TIMER_VOPBUSY + 30)							/** 语音结束检测延时时间自增上限 **/
//#define TIMER_VOPBUSYFIXED  ((TIMER_VOPBUSYMAX) + 10)	/** 语音结束检测延时时间固定值 **/

#define TIMER_VOPFAULTBEGIN  10							/** 语音结束检测异常时间自增上限 **/
#define TIMER_VOPFAULTEND  (TIMER_VOPFAULTBEGIN + 8000) /** 语音结束检测异常时间自增上限 **/

#define CTIMER_CALIB (2000)	/** 进入校正状态需要持续按下按键的时间, 斩定为2s **/

/***************** 按键 **************************/
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
#define CKEY_SET 	(0x12)		//门后的设置键
#define CKEY_VOP 	(0x13)		//门后的语音键
/*************************************************/
#define CPSWORD_MAXLEN	(30 + 2)
#define CPSWORD_MINLEN	(4)
#define CPSWORD_MAXCNT	(11)
#define CPSWORDLEN	(CPSWORD_MAXLEN - 10)
#define	CPSWORDCNT	(6 + 1)		/** 第0条密码保留 **/
/*************************************************/
#define	CRECORD_MAXLEN	(16)
#define	CRECORD_MAXCNT	(1024)
/*******************************************************************************/
#define CBLUETOOTH_MAXLEN	(30 + 2)		//分配空间(单项最大值)
#define CBLUETOOTH_MINLEN	(4)
#define CBLUETOOTH_MAXCNT	(11)			//分配空间(项数最大值)
#define CBLUETOOTHLEN	(CBLUETOOTH_MAXLEN - 10)	//实际
#define	CBLUETOOTHCNT	(6 + 1)		/** 第0条位置保留 **/

#define	AT_CMD_MAXLEN	(32)	//AT命令的长度


#define	CCMD_BTHEAD	(0x5a)
#define	CCMD_ENROLL	(0x5a)
#define	CCMD_MATCH	(0xa5)
/*******************************************************************************/
//#define	CTOTALADDRESS	1024


/**************************** 指纹传感器命令及返回 *****************************/
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


/** 表达数据传输是否正常 **/
#define	CMD_ACK_OK (0)		//正常的无错返回
#define	CMD_ACK_SUM_ERR (2)
//#define	CMD_EROLL_AGAIN (0x8301)	//已注册(重复注册)
//#define	CMD_ACK_XOR_ERR (1)

/*******************************************************************************
 * 指纹处理结果 
 *******************************************************************************/
#define	CRESP_ERR_SUCCESS 			(0)
#define	CRESP_ERR_FAIL 				(1)
/** 匹配结果 **/
#define	CRESP_ERR_IDENTIFY 				(0x12)		/** 1:N比对失败 **/
#define	CRESP_ERR_ALL_TMPL_EMPTY 		(0x15)		/** 不存在已登记模板 **/
#define	CRESP_ERR_BAD_QUALITY 			(0x21)		/** 图像质量不好 **/
#define	CRESP_ERR_BAD_TIMEOUT 			(0x23)		/** 超时 **/
#define	CRESP_GD_NEED_RELEASE_FINGER	(0xFFF4)	/** 离开手指 **/
/** 录入结果 **/

// 1. 0x00 - 指令处理成功
// 2. 0x01 - 指令处理失败
#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//指定的Template号码无效
#define CRESP_ERR_TMPL_NOT_EMPTY	(0x14)		//指定号码中已存在Template
#define	CRESP_GD_NEED_FIRST_SWEEP	(0xFFF1)	/** 第一次指纹输入等待 **/
#define	CRESP_GD_NEED_SECOND_SWEEP	(0xFFF2)	/** 第二次指纹输入等待 **/
#define	CRESP_GD_NEED_THIRD_SWEEP	(0xFFF3)	/** 第三次指纹输入等待 **/
//#define	CRESP_ERR_BAD_TIMEOUT 			(0x23)	//Timeout时间内没有检测到指纹的输入
//#define	CRESP_ERR_BAD_QUALITY 			(0x21)	/** 指纹图像质量不好 **/
#define	CRESP_ERR_GENERALIZE 		(0x30)		/** 登记的Template的制作失败 **/

/** 删除结果 **/
#define CRESP_ERR_TMPL_EMPTY		(0x13)		//指定的号码中不存在指纹模板数据
#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//指定的Template号码无效

/** 查询状态 **/
#define CRESP_GD_TEMPLATE_NOT_EMPTY	(0x01)		//指定的号码中不存在指纹模板数据
#define CRESP_GD_TEMPLATE_EMPTY		(0x00)		//指定的Template号码无效

/** 查询完整性 **/
//#define CRESP_ERR_TMPL_EMPTY		(0x13)		//指定的号码中不存在指纹模板数据
//#define CRESP_ERR_INVALID_TMPL_NO	(0x60)		//指定的Template号码无效
/***************************************************************/
/****/
#define	CINVALIDID	0xffff		/** 无效id号 **/
/*******************************************************************************/
#define	CFINGER_MAXCNT	512

//#define	CTOTALFINGER	290


/** 其中的x是指纹传感器上对应的id号(与byte图一致) **/
#define	MUSER_ID_BASE	(9)
#define	MUSER_ID_MAX	(290)	/** 指纹索引条目数, 也是指纹模板总数 **/
//#define	MUSER_ID_MAX	(CTOTALFINGER)	/** 指纹索引条目数, 也是指纹模板总数 **/

#define	MisSUPER(x) ((x) == 0)
#define	MisADMIN(x) ((x) < (MUSER_ID_BASE))
#define	MisUSER(x) 	(((x) >= (MUSER_ID_BASE)) && ((x) < (MUSER_ID_MAX)))
/** 指纹id转换成用户id **/
#define	M2UserID(x) ((x) - (MUSER_ID_BASE))	// x ==> UserID
#define	M2AdminID(x) (x)				// x ==> AdminID

//#define CADC1_DR_ADDRESS    ((u32)0x4001244C)

//#define	CFLASHSTARTADDR (0x00027C00)		/** the 256th Flash Page(512byte/page) **/
#define	CFLASHSTARTADDR (0x00018000)	/** the last 32kB(96KB-128KB) **/

#define CMAX_PSERR 6		//最大密码失败次数

#define CMAX_FPBUF 32		//最大指纹命令缓冲长度
#define CMAX_PSBUF 32		//最大密码缓冲长度
#define CMAX_BTBUF 32		//最大蓝牙数据缓冲长度
/*******************************************************************************/
//#define	CFLASH_PAGE_SIZE		256			//1024 似乎用不上了

//#define	CFLASH_ADC_VALUE		4			/** 4..5 **/
//#define	CFLASH_ADC_FLAG			6
//#define	CFLASH_RESET_VALUE		8
//#define	CFLASH_INDUCTION_POWER		12		/** 指纹头感应上电标志A5/A0作为有或无，其它为无效也视作有感应上电标志 **/
//		#define CINDUCTION_POWER_DISABLE	0xA0		/** 无指纹上电功能(默认有) **/
//		#define CINDUCTION_POWER_ENABLE		0xA5		/** 有指纹上电功能(默认有) **/
#define	CFLASH_PASSWD_ERR		4		/** 输入错误密码次数 **/
//#define	CFLASH_PASSWD_START		32
//#define	CFLASH_FPIndex_START	320

#define	CENTRYFLAG_BUSY	0xa0	/** 表示指定的项已被使用。实际应用中只要与0xF0相与等于0xA0即可 **/
#define	CENTRYFLAG_IDLE	0x0		/** 表示指定的项未被使用, 实际应用中只要与0xF0相与不是0xA0即可 **/

#define	MAGIC_SIZE 4
/*******************************************************************************/
//语音控制索引
#define	CVOPID_RESET	0

#define	CVOPID_SILENT	1
#define	CVOPID_BASE	1

#define	CVOPID_BEEP		(CVOPID_BASE + 1)
#define	CVOPID_Hao		(CVOPID_BASE + 2)	//号
#define	CVOPID_Xiao		(CVOPID_BASE + 3)	//效(当前没用到)
#define	CVOPID_Wu		(CVOPID_BASE + 4)	//无(当前没用到)
#define	CVOPID_asterisk	(CVOPID_BASE + 5)	//*
#define	CVOPID_pound	(CVOPID_BASE + 6)	//#
#define	CVOPID_Key		(CVOPID_BASE + 7)	//键
#define	CVOPID_Had		(CVOPID_BASE + 8)	//已(当前没用到)
#define	CVOPID_Full		(CVOPID_BASE + 9)	//满
#define	CVOPID_Press	(CVOPID_BASE + 10)	//按
#define	CVOPID_Please	(CVOPID_BASE + 11)	//请
#define	CVOPID_AGAIN	(CVOPID_BASE + 12)	//再
#define	CVOPID_Ci		(CVOPID_BASE + 13)	//次(当前没用到)
#define	CVOPID_PSWORD	(CVOPID_BASE + 14)	//密码
#define	CVOPID_PASSED	(CVOPID_BASE + 15)	//通过
#define	CVOPID_INPUT	(CVOPID_BASE + 16)	//输入
#define	CVOPID_FINGER	(CVOPID_BASE + 17)	//手指
#define	CVOPID_ADD		(CVOPID_BASE + 18)	//添加
#define	CVOPID_DEL		(CVOPID_BASE + 19)	//删除
#define	CVOPID_SET		(CVOPID_BASE + 20)	//设置(当前没用到)
#define	CVOPID_SYSTEM	(CVOPID_BASE + 21)	//系统(当前没用到)
#define	CVOPID_UNLOCK	(CVOPID_BASE + 22)	//解锁(当前没用到)
#define	CVOPID_COMPARISON	(CVOPID_BASE + 23)	//验证(比对)
#define	CVOPID_SUCESS		(CVOPID_BASE + 24)	//成功
#define	CVOPID_FAILED		(CVOPID_BASE + 25)	//失败
#define	CVOPID_FPRINT		(CVOPID_BASE + 26)	//指纹
#define	CVOPID_LOGIN		(CVOPID_BASE + 27)	//登录
#define	CVOPID_Doing		(CVOPID_BASE + 28)	//正在...	
#define	CVOPID_Wait			(CVOPID_BASE + 29)	//稍候
#define	CVOPID_CLEARED		(CVOPID_BASE + 30)	//清空
#define	CVOPID_INPUT2		(CVOPID_BASE + 31)	//2
#define	CVOPID_FUNCTION		(CVOPID_BASE + 32)	//功能
#define	CVOPID_CONFIRM		(CVOPID_BASE + 33)	//确认
#define	CVOPID_CANCEL		(CVOPID_BASE + 34)	//取消
#define	CVOPID_USER			(CVOPID_BASE + 35)	//用户
#define	CVOPID_ALL			(CVOPID_BASE + 36)	//所有
#define	CVOPID_Doing2		(CVOPID_BASE + 37)	//8
#define	CVOPID_VOICE		(CVOPID_BASE + 38)	//语音
#define	CVOPID_ON		(CVOPID_BASE + 39)	//开启
#define	CVOPID_OFF		(CVOPID_BASE + 40)	//关闭
#define	CVOPID_ALARM		(CVOPID_BASE + 41)	//警报声.......
#define	CVOPID_ID		(CVOPID_BASE + 42)	//编号
#define	CVOPID_LOCK		(CVOPID_BASE + 43)	//锁定
#define	CVOPID_OVER		(CVOPID_BASE + 44)	//结束
#define	CVOPID_RETURN		(CVOPID_BASE + 45)	//返回
#define	CVOPID_PAGEUP		(CVOPID_BASE + 46)	//上翻
#define	CVOPID_PAGEDN		(CVOPID_BASE + 47)	//下翻
#define	CVOPID_PLEASEPRESS	(CVOPID_BASE + 48)	//请按
#define	CVOPID_INITIAL		(CVOPID_BASE + 49)	//初始化
#define	CVOPID_DIFFERENT	(CVOPID_BASE + 50)	//不一致
#define	CVOPID_MANAGER		(CVOPID_BASE + 51)	//管理员
#define	CVOPID_ANOTHER		(CVOPID_BASE + 52)	//另一枚
#define	CVOPID_CONTINUE		(CVOPID_BASE + 53)	//继续操作
#define	CVOPID_ALLDATA		(CVOPID_BASE + 54)	//全部数据
#define	CVOPID_RESTORE		(CVOPID_BASE + 55)	//恢复出厂
#define	CVOPID_FORYOURSAFTY		(CVOPID_BASE + 56)	//为了你的安全
#define	CVOPID_TURNHANDLE		(CVOPID_BASE + 57)	//转动把手开门
#define	CVOPID_POWERLOW		(CVOPID_BASE + 58)	//电量低，请更换电池
#define	CVOPID_WELCOME		(CVOPID_BASE + 59)	//欢迎使用华园指纹锁

#define	CVOPID_FINAL		(CVOPID_WELCOME)	//结尾


#define	CVOP_VOL0		0xe0
//...
#define	CVOP_VOL7		0xe7
#define	CVOP_LOOP		0xf2	//play voice again and again
#define	CVOP_STOP		0x00	//stop playing
/*******************************************************************************/

/*******************************************************************************/
#endif
