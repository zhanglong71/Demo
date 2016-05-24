#ifndef	__L_OLED_H__
#define	__L_OLED_H__
/**************************/

//#define SIZE 16

#define COLED_MaxCol	128
#define COLED_MaxRow	64
/**************************/
#define	MOLED_RST_H()	 (GPIO_WriteHigh(GPIOE, GPIO_PIN_0))
#define	MOLED_CS_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_0))
#define	MOLED_DC_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_2))
#define	MOLED_SCL_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_3))	//D0
#define	MOLED_SIN_H()	 (GPIO_WriteHigh(GPIOD, GPIO_PIN_4))	//D1

#define OLED_RST_PORT       GPIOE
#define OLED_CS_PORT       GPIOD
#define OLED_DC_PORT       GPIOD
#define OLED_SCL_PORT       GPIOD
#define OLED_SIN_PORT       GPIOD

#define OLED_RST_PIN        GPIO_PIN_0
#define OLED_CS_PIN       GPIO_PIN_0
#define OLED_DC_PIN       GPIO_PIN_2
#define OLED_SCL_PIN       GPIO_PIN_3
#define OLED_SIN_PIN       GPIO_PIN_4

/*******************************************************************************/
void OLED_SPI_Init(void);
void OLED_SPI_DeInit(void);
void OLEDReset_IC(void);
void OLEDInit_IC(void);
void OLED_POWEROFF(void);

void All_Screen(u8 __data);
//void OLED_SetPageData(u8 y, u8 __data);
void OLED_SetPosData(u8 x, u8 y, u8 __data);
void OLED_SetMatrixData(u8 x, u8 y, u8 __width, u8 __high, u8 __data);
void OLED_cleanPages(u8 _from, u8 _to);

//void OLED_ShowChar(u8 x, u8 y, u8 ch);
void OLED_ShowChar8x16(u8 x, u8 y, u8 ch);
void OLED_ShowChar12x24(u8 x, u8 y, u8 no);
void OLED_ShowRawData(u8 x, u8 y, u8 ch);
//void OLED_ShowString(u8 x, u8 y, u8 *ch);
void OLED_ShowString8x16(u8 x, u8 y, u8 *ch);
void OLED_ShowString12x24(u8 x, u8 y, u8 *ch);
//void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size2);
void OLED_ShowNum8x16(u8 x, u8 y, u32 num, u8 len, u8 size2);
void OLED_ShowNum12x24(u8 x, u8 y, u32 num, u8 len, u8 size2);
void OLED_ShowChinese(u8 x,u8 y,u8 no);
void OLED_ShowChinese16x16(u8 x, u8 y, u8 no);
void OLED_ShowChinese24x24(u8 x, u8 y, u8 no);

//void OLED_SetPosData16x16(u8 x, u8 y, u8 __data);
//void OLED_SetPosData24x24(u8 x, u8 y, u8 __data);
/*******************************************************************************/
void DISP_idle(void);

void DISP_line2nd_menu_IDxxSel(u8 _sel, u8 _total);
void DISP_line2nd_menu_IDxxxSel(u8 _sel, u8 _total);
/*************密码开锁**********************************************************/
void DISP_line1st_psword(void);
void DISP_line2nd_psword(void);
void DISP_line3rd_psword(u8 psNO);
//void DISP_line3rd_data(u8 psNO, u8 ch);
void DISP_line3rd_array(u8 array[], u8 len);

void DISP_line2nd_PSword_ID(u8 _idx);
//void DISP_line2nd_PSWD_ID(u8 _idx);
void DISP_line2nd_PFuser_ID( u8 _idx);
void DISP_line2nd_PFadmin_ID( u8 _idx);

void DISP_PSverify_line3rd_succ(u8 _succ);
/*************菜单设置**********************************************************/

void DISP_line1st_authen_manager(void);

void DISP_line1st_menu(void);
void DISP_line2nd_finger(void);
void DISP_line3rd_menu_select(void);
//void DISP_line3rd_menu_confirm(void);
void DISP_line3rd_menu_confirm(u8 __row);
void DISP_line3rd_menu_enable(u8 __row);

void DISP_L1_menu(int _menuNO);
void DISP_L2_menu_pwmng(int _menuNO);
void DISP_menu_MNGPW_line1st(void);
void DISP_line3rd_addDoing(u8 __row);
/**************指纹设置*********************************************************/
void DISP_menu_MNGFP_line1st(void);
void DISP_L2_menu_fpmng(u8 _menuNO);

void DISP_L3_menu_line1st_addmanager(void);
void DISP_L3_menu_line1st_delmanager(void);

void DISP_L3_menu_line1st_adduser(void);
void DISP_L3_menu_line1st_deluser(void);

void DISP_menu_line2nd_PP(u8 __line);

void DISP_L3_menu_line3rd_addSucc(void);

#define	DISP_L3_menu_line2nd_confirmDelFingprint	DISP_L3_menu_line2nd_confirmDelPsword
/**************密码设置*********************************************************/
//void DISP_menu_MNGPW_line1st_addPsword(void);
//void DISP_menu_MNGPW_line1st_delPsword(void);
void DISP_L3_menu_line1st_addPsword(void);
void DISP_L3_menu_line1st_delPsword(void);
void DISP_L3_menu_line2nd_confirmDelPsword(void);

void DISP_L3_menu_line2nd_inputPsword(void);
void DISP_L3_menu_line2nd_ImgPsword(void);

void DISP_line3rd_empty(u8 __line);
void DISP_line3rd_full(u8 __line);
/**************蓝牙设置*********************************************************/
void DISP_menu_MNGBT_line1st(void);
void DISP_menu_MNGBT_line2nd_addPsword(void);
void DISP_menu_MNGBT_line2nd_delPsword(void);

void DISP_L2_menu_btmng(u8 _menuNO);

void DISP_L3_menu_line1st_addBluetooth(void);
void DISP_L3_menu_line1st_delBluetooth(void);

void DISP_line2nd_Bluetooth_ID(u8 _idx);
/*******************************************************************************/
void DISP_line3rd_AddSucc(void);
void DISP_line3rd_AddFail(void);
void DISP_line3rd_delSucc(u8 __row);
void DISP_line3rd_delFail(u8 __row);
void DISP_line3rd_delDoing(u8 __row);

/*************系统设置**********************************************************/
void DISP_menu_MNGSYS_line1st(void);


void DISP_menu_MNGSYS_line1st_OpenMode(void);
void DISP_menu_MNGSYS_line2nd_RestoreConfirm(void);

void DISP_menu_MNGSYS_line2nd_RestoreDoing(void);
//void DISP_menu_MNGSYS_line3rd_RestoreDoing(u8 __row);
void DISP_menu_MNGSYS_line3rd_Waiting(u8 __row);
void DISP_menu_MNGSYS_line3rd_succ(u8 __row);
void DISP_line3rd_menu_enableStat(u8 __enable);
void DISP_L2_menu_sysmng(int _menuNO);
void DISP_L2_menu_OpenModemng(int _menuNO);
void DISP_GetOrSetDateTime(u8 __setget);
void DISP_SetDateTime_Blink(u8 __order, u8 __invert, u8 __data);

void DISP_open_FPempty(void);
void DISP_PowerOn(void);
void DISP_lowPowerCheck(void);
void DISP_lowPower(void);
void DISP_version(void);

void DISP_DateTime(u8 *__datetime);
void DISP_RecordId(u16 __idx, u8 *__record);
void DISP_NoRecord(void);
/*************系统查询**********************************************************/
void DISP_menu_Query_line1st(void);
//void DISP_version(void);

void DISP_L2_menu_mngQuery(u8 _menuNO);
/*******************************************************************************/
#endif
