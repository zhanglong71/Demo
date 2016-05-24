#ifndef __MACRO_H__
#define __MACRO_H__
/*******************************************************************************/
/***************************** 数组大小 ****************************************/
#define	MTABSIZE(x)	(sizeof(x)/sizeof(x[0]))
/*******************************************************************************/
//#define	MIRQ_enable()	__enable_interrupt()		//中断使能
//#define	MIRQ_disable()	__disable_interrupt()		//中断禁止
/********************************** 按键背光LED ********************************/

/**************************** 指纹头供电及感应上电检测 *************************/
#if	0
#define	MFingerPower_off() (GPIO_WriteLow(GPIOG, GPIO_PIN_5))
#define	MFingerPower_on() (GPIO_WriteHigh(GPIOG, GPIO_PIN_5))
#endif
/** 读取指纹头感应状态 **/
//#define	MFP_WakeCheck() (GPIO_ReadInputPin(GPIOE, GPIO_PIN_4))
/********************************** 主IC供电线路 *******************************/
#if	0
#define	MMasterPower_off() do{GPIO_WriteLow(GPIOB, GPIO_PIN_9);}while(GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_9))
#define	MMasterPower_on() (GPIO_WriteHigh(GPIOB, GPIO_PIN_9))
#endif

/** 低电检测 **/
#define	MSYSPOWER_OFF(x) do{(GPIO_WriteLow(GPIOF, GPIO_PIN_0));}while(0)
#define	MSYSPOWER_ON(x) do{(GPIO_WriteHigh(GPIOF, GPIO_PIN_0));}while(0)
#define	MSYSPOWER_INIT(x) do{GPIO_Init(GPIOF, GPIO_PIN_0, GPIO_MODE_OUT_PP_HIGH_FAST );}while(0)

#define	MSYSPOWER_GET() (GPIO_ReadOutputDataBit(GPIOB, GPIO_PIN_7))
 /*****************************************************************/
#define MBCD2HEX(_bcd)	((((_bcd >> 4) & 0x0f) * 10) + (_bcd & 0x0f))
#define MHEX2BCD(_hex)	((((_hex/10) << 4) & 0xf0) | (_hex % 10))
/********************************** 操作指示灯 *********************************/
/*
 * D1- GPF.5/D2- GPF.6/D3-GPB.6
 * D4- GPI.0/D5- GPE.3/D6-GPG.1
 * D7- GPA.3/D8- GPD.7/D9-GPG.4
 * D10-GPG.3/D11-GPB.0/D12-GPG.2
 */
/*******LED.NO----------GPIO----------OVERLAY********/
#if	1
	#define LED8_PIN        GPIO_PIN_5    //LED1_PIN
	#define LED11_PIN        GPIO_PIN_6    //LED2_PIN
	#define LED7_PIN        GPIO_PIN_6    //LED3_PIN
	#define LED3_PIN        GPIO_PIN_0    //LED4_PIN
	#define LED9_PIN        GPIO_PIN_3    //LED5_PIN
	#define LED6_PIN        GPIO_PIN_1    //LED6_PIN
	#define LED12_PIN        GPIO_PIN_3    //LED7_PIN
	#define LED10_PIN        GPIO_PIN_7    //LED8_PIN
	#define LED2_PIN        GPIO_PIN_4    //LED9_PIN
	#define LED1_PIN        GPIO_PIN_3    //LED10_PIN
	#define LED4_PIN        GPIO_PIN_0    //LED11_PIN
	#define LED5_PIN        GPIO_PIN_2    //LED12_PIN
	#define LED8_PORT       GPIOF    //LED1_PORT
	#define LED11_PORT       GPIOF    //LED2_PORT
	#define LED7_PORT       GPIOB    //LED3_PORT
	#define LED3_PORT       GPIOI    //LED4_PORT
	#define LED9_PORT       GPIOE    //LED5_PORT
	#define LED6_PORT       GPIOG    //LED6_PORT
	#define LED12_PORT       GPIOA    //LED7_PORT
	#define LED10_PORT       GPIOD    //LED8_PORT
	#define LED2_PORT       GPIOG    //LED9_PORT
	#define LED1_PORT       GPIOG    //LED10_PORT
	#define LED4_PORT       GPIOB    //LED11_PORT
	#define LED5_PORT       GPIOG    //LED12_PORT
#else	//小板修改后的对应关系
	#define LED1_PIN        GPIO_PIN_0
	#define LED2_PIN        GPIO_PIN_3
	#define LED3_PIN        GPIO_PIN_4
	#define LED4_PIN        GPIO_PIN_5
	#define LED5_PIN        GPIO_PIN_1
	#define LED6_PIN        GPIO_PIN_2
	#define LED7_PIN        GPIO_PIN_6
	#define LED8_PIN        GPIO_PIN_7
	#define LED9_PIN        GPIO_PIN_6
	#define LED10_PIN        GPIO_PIN_0
	#define LED11_PIN        GPIO_PIN_3
	#define LED12_PIN        GPIO_PIN_3
	
	#define LED1_PORT       GPIOB
	#define LED2_PORT       GPIOG
	#define LED3_PORT       GPIOG
	#define LED4_PORT       GPIOF
	#define LED5_PORT       GPIOG
	#define LED6_PORT       GPIOG
	#define LED7_PORT       GPIOB
	#define LED8_PORT       GPIOD
	#define LED9_PORT       GPIOF
	#define LED10_PORT       GPIOI
	#define LED11_PORT       GPIOE
	#define LED12_PORT       GPIOA
#endif
/********************** BLUETOOTH **************************/
#define BLE_POWER_CTR_PORT       GPIOC
#define BLE_POWER_CTR_PIN        GPIO_PIN_1

#define BLE_AT_CTR_PORT       GPIOC
#define BLE_AT_CTR_PIN        GPIO_PIN_2

#define BLE_LINK_CHK_PORT       GPIOC
#define BLE_LINK_CHK_PIN        GPIO_PIN_3

#define BLE_LINKED(x)  (GPIO_ReadInputPin(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN) == 1)  //高有效
/************************************ 锁体电机 *********************************/
#if 0
#define	MLock()  do{\
		GPIO_WriteLow(GPIOF, GPIO_PIN_4);\
		GPIO_WriteHigh(GPIOF, GPIO_PIN_3);\
				}while(0)
#define	MUnLock()  do{\
	GPIO_WriteHigh(GPIOF, GPIO_PIN_4);\
	GPIO_WriteLow(GPIOF, GPIO_PIN_3);\
				}while(0)
#define	MLockPWR_off()  do{\
	GPIO_WriteLow(GPIOF, GPIO_PIN_4);\
	GPIO_WriteLow(GPIOF, GPIO_PIN_3);\
			}while(0)
#endif

/***************************锁体电机MOTO****************************************/
#define MOTO1_PORT       GPIOF
#define MOTO2_PORT       GPIOF

#define MOTO1_PIN        GPIO_PIN_3
#define MOTO2_PIN        GPIO_PIN_4 
/******************************** OLED功能 **************************************/
#if 1
#define	MOLED_RST_H()	 (GPIO_WriteHigh(GPIOE, GPIO_PIN_0))
#define	MOLED_CS_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_0))
#define	MOLED_DC_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_2))
#define	MOLED_SCL_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_3))	//D0
#define	MOLED_SIN_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_4))	//D1

#define	MOLED_RST_L()	 (GPIO_WriteLow(GPIOE, GPIO_PIN_0))
#define	MOLED_CS_L()	 (GPIO_WriteLow(GPIOD, GPIO_PIN_0))
#define	MOLED_DC_L()	 (GPIO_WriteLow(GPIOD, GPIO_PIN_2))
#define	MOLED_SCL_L()	 (GPIO_WriteLow(GPIOD, GPIO_PIN_3))	//D0
#define	MOLED_SIN_L()	 (GPIO_WriteLow(GPIOD, GPIO_PIN_4))	//D1
#endif
/***************************** 指纹头感应上电功能 ********************************/

/************************************ 语音控制 ***********************************/
#define VOICE_DATA_PORT       GPIOA
#define VOICE_RST_PORT       GPIOF
//#define VOICE_BUSY_PORT        GPIOC

#define VOICE_DATA_PIN        GPIO_PIN_6
#define VOICE_RST_PIN        GPIO_PIN_7 
//#define VOICE_BUSY_PIN        GPIO_PIN_1
/****** Note: 注意，通过三极管逻辑反向 ******/
#define	MVOPDATA_H() 	(GPIO_WriteLow(VOICE_DATA_PORT, VOICE_DATA_PIN))	/** 通过三极管逻辑反向 **/
#define	MVOPDATA_L()	(GPIO_WriteHigh(VOICE_DATA_PORT, VOICE_DATA_PIN))	/** 通过三极管逻辑反向 **/

#define	MVOPRESET_H()	(GPIO_WriteLow(VOICE_RST_PORT, VOICE_RST_PIN)) 	/** 通过三极管逻辑反向 **/
#define	MVOPRESET_L()	(GPIO_WriteHigh(VOICE_RST_PORT, VOICE_RST_PIN))	/** 通过三极管逻辑反向 **/

//#define MVOP_BUSYCHECK()  (GPIO_ReadInputPin(VOICE_BUSY_PORT, VOICE_BUSY_PIN) == 0) //低有效
/*********************************************************************************/
//only for receive data from uart1(interrupt context)
#define	MFPACK_FIFO_IN(x) do{	\
	g_uart1buf[0] = ((g_uart1buf[0] >= CMAX_PSBUF - 1)? (CMAX_PSBUF - 1):(g_uart1buf[0] + 1));	\
	g_uart1buf[g_uart1buf[0]] = x;	\
	}while(0)
//only for get data from uart1 received buffer(main context)
#define	MFPACK_FIFO_OUT(x) do{	\
	g_uart1buf[1] = ((g_uart1buf[1] >= CMAX_PSBUF - 1)? (CMAX_PSBUF - 1):(g_uart1buf[1] + 1));	\
	x = g_uart1buf[g_uart1buf[1]];	\
	}while(0)

#define	MFPACK_FIFO_GETLEN(x)	(g_uart1buf[0] - g_uart1buf[1])
#define	MFPACK_FIFO_CLEAN(x)	do{g_uart1buf[0] = g_uart1buf[1] = 1;}while(0)
/*******************************************************************************/
//only for receive data from uart3(interrupt context)
#define	MBTACK_FIFO_IN(x) do{	\
	g_uart3buf[0] = ((g_uart3buf[0] >= CMAX_BTBUF - 1)? (CMAX_BTBUF - 1):(g_uart3buf[0] + 1));	\
	g_uart3buf[g_uart3buf[0]] = x;	\
	}while(0)
//only for get data from uart1 received buffer(main context)
#define	MBTACK_FIFO_OUT(x) do{	\
	g_uart3buf[1] = ((g_uart3buf[1] >= CMAX_BTBUF - 1)? (CMAX_BTBUF - 1):(g_uart3buf[1] + 1));	\
	x = g_uart3buf[g_uart3buf[1]];	\
	}while(0)

#define	MBTACK_FIFO_GETLEN(x)	(g_uart3buf[0] - g_uart3buf[1])
#define	MBTACK_FIFO_CLEAN(x)	do{g_uart3buf[0] = g_uart3buf[1] = 1;}while(0)

#define	MCOMMENT(x)
/*******************************************************************************/

#endif
