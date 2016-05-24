
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "arch.h"
#include "l_oled.h"
#include "l_oledfont.h"

/*********************************************************************
 * 以16MHz的内部时钟，每一条指令的速度0.625us. 
 * 通过网络上的了解， 调用库函数比直接操作寄存器的的效率低很多，速度不会快到对方接收不到的地步
 *********************************************************************/

/*********************************************************************/
void  SPI_RST_H(void) {GPIO_WriteHigh(OLED_RST_PORT, OLED_RST_PIN);}
void  SPI_CS_H(void)  {GPIO_WriteHigh(OLED_CS_PORT, OLED_CS_PIN);}
void  SPI_DC_H(void)  {GPIO_WriteHigh(OLED_DC_PORT, OLED_DC_PIN);}
void  SPI_SCL_H(void) {GPIO_WriteHigh(OLED_SCL_PORT, OLED_SCL_PIN);}	//D0
void  SPI_SIN_H(void) {GPIO_WriteHigh(OLED_SIN_PORT, OLED_SIN_PIN);}	//D1

void  SPI_RST_L(void) {GPIO_WriteLow(OLED_RST_PORT, OLED_RST_PIN);}
void  SPI_CS_L(void)  {GPIO_WriteLow(OLED_CS_PORT, OLED_CS_PIN);}
void  SPI_DC_L(void)  {GPIO_WriteLow(OLED_DC_PORT, OLED_DC_PIN);}
void  SPI_SCL_L(void) {GPIO_WriteLow(OLED_SCL_PORT, OLED_SCL_PIN);}	//D0
void  SPI_SIN_L(void) {GPIO_WriteLow(OLED_SIN_PORT, OLED_SIN_PIN);}	//D1
/*********************************************************************/
void OLED_SPI_Init(void)
{
	GPIO_Init(OLED_RST_PORT,OLED_RST_PIN,GPIO_MODE_OUT_PP_HIGH_FAST );
	GPIO_Init(OLED_CS_PORT,OLED_CS_PIN,GPIO_MODE_OUT_PP_HIGH_FAST );

	GPIO_Init(OLED_DC_PORT,OLED_DC_PIN,GPIO_MODE_OUT_PP_HIGH_FAST );
	GPIO_Init(OLED_SCL_PORT,OLED_SCL_PIN,GPIO_MODE_OUT_PP_HIGH_FAST );
	GPIO_Init(OLED_SIN_PORT,OLED_SIN_PIN,GPIO_MODE_OUT_PP_HIGH_FAST );

	SPI_RST_L();
}

/*********************************************************************
 * 进入低功耗状态。(这些端口设为浮空输入端口) 
 *********************************************************************/
void OLED_SPI_DeInit(void)
{
	#if	1
	GPIO_Init(OLED_RST_PORT, OLED_RST_PIN, GPIO_MODE_IN_FL_NO_IT );
	GPIO_Init(OLED_CS_PORT, OLED_CS_PIN, GPIO_MODE_IN_FL_NO_IT );

	GPIO_Init(OLED_DC_PORT, OLED_DC_PIN, GPIO_MODE_IN_FL_NO_IT );
	GPIO_Init(OLED_SCL_PORT, OLED_SCL_PIN, GPIO_MODE_IN_FL_NO_IT );
	GPIO_Init(OLED_SIN_PORT, OLED_SIN_PIN, GPIO_MODE_IN_FL_NO_IT );
	#endif
}
/*********************************************************************/

/*********************************************************************
 * SH1106复位
 * RST复位(L=50ms)
 *********************************************************************/
void OLEDReset_IC(void)
{
#if	1
	SPI_RST_H();
	delaySpin_ms(50);
	SPI_RST_L();
	delaySpin_ms(50);
	SPI_RST_H();
	delaySpin_ms(50);
#endif
}
/*********************************************************************
 *
 * 写命令函数
 * 
 *********************************************************************/
void Write_Command(u8 command)
{
	u8 i,value;
	value = command;
	MOLED_CS_L();
	MOLED_DC_L();		//command
	for(i = 0; i < 8; i++)
	{
		MOLED_SCL_L();nop(); nop();
		if(value & 0x80) MOLED_SIN_H(); else  MOLED_SIN_L();
		MOLED_SCL_H();nop(); nop();
		value <<= 1;
	}
	
	MOLED_CS_H();
	MOLED_DC_H();		//default data
}

/**********************************************
 *
 * 写命令函数
 *
 *********************************************/
void Write_Data(u8 date)
{
	u8 i,value;
	value = date;
	MOLED_CS_L();
	MOLED_DC_H();		//data
	for(i = 0; i < 8; i++)
	{
		MOLED_SCL_L();nop(); nop();
		if(value & 0x80) MOLED_SIN_H(); else  MOLED_SIN_L();
		MOLED_SCL_H();nop(); nop();
		value <<= 1;
	}
	MOLED_CS_H();
}

/******************************************************
//
//初始化IC函数
//
******************************************************/
const u8 oledCfg[] = {
	0xAE,0x02,0x10,0x40,
	0x81,0xA5,0xA1,0xC8,
	0xA6,0xA8,0x3f,0xD3,
	0x00,0xD5,0x80,0xD9,
	0xF1,0xDA,0x12,0xDB,
	0x40,0x20,0x02,0x8D,
	0x14,0xA4,0xA6,0xAF,
};

void OLEDInit_IC(void)
{
 	int i = 0;
	//for(i = 0; i < sizeof(oledCfg)/sizeof(oledCfg[0]);i++) {
	for(i = 0; i < MTABSIZE(oledCfg);i++) {	
		Write_Command(oledCfg[i]);
	} 
}

void OLED_POWEROFF(void)
{
	Write_Command(0xAE);
}

/***********************************************************
 * 设置坐标
 * 从SH1106的数据手册得取值范围(列号x: 0..128，页号page: 0..7)
 ***********************************************************/
void OLED_SetPos(u8 x, u8 y)
{
	Write_Command(0xB0 + y);
	Write_Command(((x & 0xF0) >> 4) | 0x10);
	Write_Command((x & 0x0F) | 0x02);
}

/** 对一个8x8矩阵空间赋值 **/
void OLED_SetPosData(u8 x, u8 y, u8 __data)
{
	u8 cNO;
	Write_Command(0xB0 + y);
	Write_Command(((x & 0xF0) >> 4) | 0x10);
	Write_Command((x & 0x0F) | 0x02);
	for(cNO = x; ((cNO < x + 8) && (cNO < 128)); cNO++) {
        Write_Data(__data);
    }
}

/*********************************************************************
 * 对一个8x8的len x width矩阵空间赋值 
 * 左上角座标(x, y)，右下角座标(x+width, y+high)
 *	(x, y)以(点,页)为单位，__high/__width以8个点为单位
 *********************************************************************/
void OLED_SetMatrixData(u8 x, u8 y, u8 __width, u8 __high, u8 __data)
{
	u8 cNO;
	u8 i;
	for(i = 0; i < __high; i++) {
		Write_Command(0xB0 + y);
		Write_Command(((x & 0xF0) >> 4) | 0x10);
		Write_Command((x & 0x0F) | 0x02);
		for(cNO = x; ((cNO < x + (__width << 3)) && (cNO < 128)); cNO++) {
    	    Write_Data(__data);
    	}
    	y++;
	}
}

/** 对指定行赋值 **/
void OLED_SetPageData(u8 __row, u8 __data)
{
	u8 colNO;
	Write_Command(0xb0 + __row);
    Write_Command(0x10);
    Write_Command(0x02);
    for(colNO = 0; colNO < 128; colNO++) {
        Write_Data(__data);
    }
}

void OLED_cleanPages(u8 _from, u8 _to)
{
	u8 pNO;
    for(pNO = _from; pNO <= _to; pNO++) OLED_SetPageData(pNO, 0);
}

/*********************************************************************
 * 偿试理解如下：2个维度设定显示内容
 * pageNO: 可理解为大行号(写入数据的8个bit可理解为大行号内的小行号) 
 * colNO: 列号
 * 利用列号自增规则Write_Data()每次写入下一个位置
 *********************************************************************/
void All_Screen(u8 __data)
{
	u8 pNO, cNO;
	for(pNO = 0; pNO < 8; pNO++) {
		Write_Command(0xb0 + pNO);
		Write_Command(0x10);
		Write_Command(0x02);
		for(cNO = 0; cNO < 128; cNO++) {
			Write_Data(__data);
		}
	}
}

/***********************************************************
 * 显示字符(OLED_ShowChar8x16/OLED_ShowChar8x16_invert)
 * 如果超出一列号，就在下一行的开始显示
 ***********************************************************/
void OLED_ShowChar8x16(u8 x, u8 y, u8 ch)
{
	u8 c = 0,i = 0;
	c = ch - ' ';
	if(x > COLED_MaxCol - 1) {
		x = 0;
		y = y + 2;
	}

	OLED_SetPos(x, y);
	for(i = 0; i < 8; i++)Write_Data(F8X16[c * 16 + i]);
	OLED_SetPos(x, y + 1);
	for(i = 0; i < 8; i++)Write_Data(F8X16[c * 16 + i + 8]);
}
void OLED_ShowChar8x16_invert(u8 x, u8 y, u8 ch)
{
	u8 c = 0,i = 0;
	c = ch - ' ';
	if(x > COLED_MaxCol - 1) {
		x = 0;
		y = y + 2;
	}

	OLED_SetPos(x, y);
	for(i = 0; i < 8; i++)Write_Data(~F8X16[c * 16 + i]);
	OLED_SetPos(x, y + 1);
	for(i = 0; i < 8; i++)Write_Data(~F8X16[c * 16 + i + 8]);
}

/***********************************************************
 * m的n次方
 ***********************************************************/
u32 oled_pow(u8 m,u8 n)
{
	u32 result = 1;
	while(n--)result *= m;
	return result;
}

/***********************************************************
 * 显示数字(OLED_ShowNum8x16/OLED_ShowNum8x16_invert)
 * num：待显示数字
 * len: 数字位数(如1000可视为4位)
 * (x, y)所在坐标
 * size2: 1个全字符宽度
 ***********************************************************/
void OLED_ShowNum8x16(u8 x, u8 y, u32 num, u8 len, u8 size2)
{
	u8 t,temp;
	u8 enshow = 0;				//有效显示标志
	for(t = 0; t < len; t++) {	//有效长度
		temp = (num/oled_pow(10, len - t - 1)) % 10;
		if(enshow == 0 && t < (len-1)) {
			if(temp==0)	{
				//OLED_ShowChar8x16(x + (size2/2) * t, y, ' ');
				OLED_ShowChar8x16(x + (size2/2) * t, y, '0');
				continue;
			} else enshow = 1;
		}
		OLED_ShowChar8x16(x + (size2/2) * t, y, temp + '0');
	}
}

void OLED_ShowNum8x16_invert(u8 x, u8 y, u32 num, u8 len, u8 size2)
{
	u8 t,temp;
	u8 enshow = 0;				//有效显示标志
	for(t = 0; t < len; t++) {	//有效长度
		temp = (num/oled_pow(10, len - t - 1)) % 10;
		if(enshow == 0 && t < (len-1)) {
			if(temp==0)	{
				//OLED_ShowChar8x16(x + (size2/2) * t, y, '0');
				OLED_ShowChar8x16_invert(x + (size2/2) * t, y, '0');
				continue;
			} else enshow = 1;
		}
		//OLED_ShowChar8x16(x + (size2/2) * t, y, temp + '0');
		OLED_ShowChar8x16_invert(x + (size2/2) * t, y, temp + '0');
	}
}

void OLED_ShowString8x16(u8 x, u8 y, u8 *ch)
{
	u8 j = 0;
	while (ch[j] != '\0') {
		OLED_ShowChar8x16(x, y, ch[j]);
		x = x + 8;
		if(x > 120) {
			x = 0;
			y = y + 2;
		}
		j++;
	}
}

//void OLED_ShowNum12x24(u8 x, u8 y, u32 num, u8 len, u8 size2)
void OLED_ShowNum12x24(u8 x, u8 y, u32 num, u8 len, u8 size2)
{
	u8 t,temp;
	u8 enshow = 0;				//有效显示标志
	for(t = 0; t < len; t++) {	//有效长度
		temp = (num/oled_pow(10, len - t - 1)) % 10;
		if(enshow == 0 && t < (len-1)) {
			if(temp==0)	{
				//OLED_ShowChar12x24(x + (size2/2) * t, y, ' ');
				OLED_ShowChar12x24(x + (size2/2) * t, y, '0');
				continue;
			} else enshow = 1;
		}
		OLED_ShowChar12x24(x + (size2/2) * t, y, temp + '0');
	}
}

void OLED_ShowString12x24(u8 x, u8 y, u8 *ch)
{
	u8 j = 0;
	while (ch[j] != '\0') {
		OLED_ShowChar12x24(x, y, ch[j]);
		x = x + 12;
		if(x > 116) {
			x = 0;
			y = y + 3;
		}
		j++;
	}
}

/***********************************************************
 * 显示位置的分辨率以8x8的点阵为单位, 
 * 显示16x16点阵字符
 * 要求是以 font16x16[][32]
 ***********************************************************/
void OLED_ShowChinese16x16(u8 x, u8 y, u8 no)
{
	u8 t;
	x <<= 3; /** x = x * 8; **/
	OLED_SetPos(x, y);
	for(t = 0; t < 16; t++) Write_Data(font16x16[no][t]);
	OLED_SetPos(x, y + 1);
	for(t = 0; t < 16; t++) Write_Data(font16x16[no][t + 16]);
}

/***********************************************************
 * 显示位置的分辨率以8x8的点阵为单位, 
 * 显示12x24点阵字符
 * 要求是以 font12x24[][36]
 ***********************************************************/
void OLED_ShowChar12x24(u8 x, u8 y, u8 ch)
{
	u8 c = 0, i = 0;
	c = ch - ' ';
	if(x > COLED_MaxCol - 1) {
		x = 0;
		y = y + 3;
	}

	OLED_SetPos(x, y);
	for(i = 0; i < 12; i++)Write_Data(font12x24[c][i]);
	OLED_SetPos(x, y + 1);
	for(i = 0; i < 12; i++)Write_Data(font12x24[c][i + 12]);
	OLED_SetPos(x, y + 2);
	for(i = 0; i < 12; i++)Write_Data(font12x24[c][i + 24]);
}

/***********************************************************
 * 显示24x24文字
 * 对sh1106而言，24x24点阵跨越3个page, 每个page对应24byte数据
 ***********************************************************/
void OLED_ShowChinese24x24(u8 x, u8 y, u8 no)
{
	u8 t;
	x <<= 3; /** x = x * 8; **/
	
	OLED_SetPos(x, y);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t]);
	OLED_SetPos(x, y + 1);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t + 24]);
	OLED_SetPos(x, y + 2);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t + 24 + 24]);

}

void OLED_ShowChinese24x24_Px_Py(u8 x, u8 y, u8 no)
{
	u8 t;
	
	OLED_SetPos(x, y);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t]);
	OLED_SetPos(x, y + 1);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t + 24]);
	OLED_SetPos(x, y + 2);
	for(t = 0; t < 24; t++)Write_Data(Hzk24x24[no][t + 24 + 24]);
}

/***********************************************************
 * 画位图bitmap
 ***********************************************************/
void OLED_DrawBMP(u8 x0, u8 y0,u8 x1, u8 y1, u8 BMP[])
{
	u16 j = 0;
	u8 x, y;

	//if(y1 % 8 == 0) y = y1/8;else y = y1/8 + 1;
	y = (y1 + 7) >> 3;
	for(y = y0; y < y1; y++) {
		OLED_SetPos(x0, y);
		for(x = x0; x < x1; x++) {
			Write_Data(BMP[j++]);
		}
	}
}
/**********************************************************/
void DISP_idle(void)
{
	//OLED_ShowString8x16(0,0,"OLED"); //?????????????????
    //OLED_ShowChinese16x16(4,0,0);	//华
	//OLED_ShowChinese16x16(6,0,1);	//园
	//OLED_ShowChinese16x16(8,0,2);	//科
	//OLED_ShowChinese16x16(10,0,3);	//技
    
	OLED_ShowChinese24x24(2,2,0);	//华
	OLED_ShowChinese24x24(5,2,1);	//园
	OLED_ShowChinese24x24(8,2,2);	//科
	OLED_ShowChinese24x24(11,2,3);	//技
	
	OLED_ShowChinese16x16(3,6,45);	//智
	OLED_ShowChinese16x16(5,6,46);	//能
	OLED_ShowChinese16x16(7,6,50);	//指
	OLED_ShowChinese16x16(9,6,51);	//纹
	OLED_ShowChinese16x16(11,6,47);	//锁
}
/**********************密码开门****************************/
void DISP_line1st_psword(void)
{
	OLED_ShowChinese16x16(4,0,17);	  //密
	OLED_ShowChinese16x16(6,0,18);	  //码
	OLED_ShowChinese16x16(8,0,52);	  //开
	OLED_ShowChinese16x16(10,0,47);	  //锁
}
void DISP_line2nd_psword(void)
{
	OLED_ShowChinese24x24(2,2,59);	  //输
	OLED_ShowChinese24x24(5,2,60);	  //入
	OLED_ShowChinese24x24(8,2,9);	  //密
	OLED_ShowChinese24x24(11,2,10);	  //码
}

/** "****________" **/
void DISP_line3rd_psword(u8 psNO)
{
	/** 显示psNO个*, 剩下的显示 **/
	int i; 
	for(i = 0; ((i < psNO) & (i < 12)); i++)OLED_ShowString8x16(((i + 2) << 3), 6, "*"); 	/** 先显示* **/
	for(; i < 12; i++)OLED_ShowString8x16(((i + 2) << 3), 6, "_");			/** 再显示_ **/
}

/***********************************************************
 * 在第3行的位置显示字符 
 * for debug
 ***********************************************************/
void DISP_line3rd_array(u8 array[], u8 len)
{
	int i;
	for(i = 0; i < len; i++) OLED_ShowNum8x16((i << 3), 6, array[i], 1, 16);
}
/***********************************************************
 * 验证管理员/请按手指
 ***********************************************************/
void DISP_line1st_authen_manager(void)
{
	OLED_ShowChinese24x24(0, 0, 63);	//验
	OLED_ShowChinese24x24(3, 0, 64);	//证
	OLED_ShowChinese24x24(6, 0, 67);	//管
	OLED_ShowChinese24x24(9, 0, 68);	//理
	OLED_ShowChinese24x24(12, 0, 69);	//员
}

/***********************************************************
 * 成功/失败
 ***********************************************************/
void DISP_PSverify_line3rd_succ(u8 _succ)
{
	u8 lineNO = 5;
	
	if(_succ == 0)lineNO = 4;
	
	OLED_ShowChinese24x24(2,lineNO, 63);	  //验
	OLED_ShowChinese24x24(5,lineNO, 64);	  //证
	if(_succ != 0) {
		OLED_ShowChinese24x24(8, lineNO, 65);	//通
		OLED_ShowChinese24x24(11, lineNO, 66);	//过
	} else {
		OLED_ShowChinese24x24(8, lineNO, 61);	//失
		OLED_ShowChinese24x24(11, lineNO, 62);	//败
	}
}


/***********************************************************
 * 第一行
 ***********************************************************/
void DISP_line1st_menu(void)
{
	OLED_ShowChinese16x16(6,0,27);	  //设
	OLED_ShowChinese16x16(8,0,28);	  //置
}

/***********************************************************
 * 第二行
 ***********************************************************/
void DISP_line2nd_finger(void)
{
	OLED_ShowString12x24(0, 2, "1."); //1
	OLED_ShowChinese24x24(3,2,7);	  //指
	OLED_ShowChinese24x24(6,2,8);	  //纹
	OLED_ShowChinese24x24(9,2,13);	  //设
	OLED_ShowChinese24x24(12,2,14);	  //置
}

void DISP_line2nd_mngpsword(void)
{
	OLED_ShowString12x24(0, 2, "2."); //2
	OLED_ShowChinese24x24(3,2,9);	  //密
	OLED_ShowChinese24x24(6,2,10);	  //码
	OLED_ShowChinese24x24(9,2,13);	  //设
	OLED_ShowChinese24x24(12,2,14);	  //置
}

void DISP_line2nd_mngbluetooth(void)
{
	OLED_ShowString12x24(0, 2, "3."); //3
	OLED_ShowChinese24x24(3,2,85);	  //蓝
	OLED_ShowChinese24x24(6,2,86);	  //牙
	OLED_ShowChinese24x24(9,2,13);	  //设
	OLED_ShowChinese24x24(12,2,14);	  //置
}

void DISP_line2nd_system(void)
{
	OLED_ShowString12x24(0, 2, "4."); //4
	OLED_ShowChinese24x24(3,2,11);	  //系
	OLED_ShowChinese24x24(6,2,12);	  //统
	OLED_ShowChinese24x24(9,2,13);	  //设
	OLED_ShowChinese24x24(12,2,14);	  //置
}

void DISP_line2nd_query(void)
{
	OLED_ShowString12x24(0, 2, "5."); //5
	OLED_ShowChinese24x24(3,2,15);	  //信
	OLED_ShowChinese24x24(6,2,16);	  //息
	OLED_ShowChinese24x24(9,2,17);	  //查
	OLED_ShowChinese24x24(12,2,18);	  //询
}

/***********************************************************
 * 第三行(返回* 2↑  8↓  确认#)
 ***********************************************************/
void DISP_line3rd_menu_select(void)
{
	OLED_ShowChinese16x16(0, 6, 21);	  		//返
    OLED_ShowChinese16x16(2, 6, 22);	  		//回
	OLED_ShowString8x16((4 << 3), 6, "*");   	//*
	OLED_ShowString8x16((5 << 3) + 4, 6, "2");  //2
	OLED_ShowChar8x16((6 << 3) + 4, 6, (u8)95 + ' ');   //<
            
	OLED_ShowString8x16((8 << 3), 6, "8");   	//8
	OLED_ShowChar8x16((9 << 3), 6, (u8)96 + ' ');   //<
    OLED_ShowChinese16x16(11, 6, 19);	  		//确
	OLED_ShowChinese16x16(13, 6, 20);	  		//认
	OLED_ShowString8x16((15 << 3), 6, "#");   	//#
}

/***********************************************************
 * 第三行[禁用(*)        启用(#)]
 ***********************************************************/
void DISP_line3rd_menu_enable(u8 __row)
{
	OLED_ShowChinese16x16(0, __row, 62);	  //禁
    OLED_ShowChinese16x16(2, __row, 13);	  //用
	OLED_ShowString8x16((4 << 3), __row, "(*)");   //(*)
	
	OLED_SetMatrixData((7 << 3), __row, 2, 2 ,0);	//clean
	
    OLED_ShowChinese16x16(9, __row, 61);	  //启
	OLED_ShowChinese16x16(11, __row, 13);	  //用
	OLED_ShowString8x16((13 << 3), __row, "(#)");   //(#)
}

/** (否(*)   是(#) ) **/
void DISP_line3rd_menu_confirm(u8 __row)
{
    OLED_ShowChinese16x16(0, __row, 24);	  //否
	OLED_ShowString8x16((2 << 3), __row, "(*)");   //*
	OLED_SetMatrixData((2 << 3) + (3 << 3), __row, 5, 2 ,0);	//clean

    OLED_ShowChinese16x16(11, __row, 23);	  //是
	OLED_ShowString8x16((13 << 3), __row, "(#)");   //#
}

/****************显示要选择的序号***********************************************/
/** (ID: 01/03) **/
void DISP_line2nd_menu_IDxxSel(u8 _sel, u8 _total)
{
	OLED_ShowString12x24((2 << 3), 2, "ID:  /"); 		//ID:
	OLED_ShowNum12x24((2 << 3)+ 36, 2, _sel, 2, 24);
	OLED_ShowNum12x24((2 << 3)+ 36 + 36, 2, _total, 2, 24);
}

/*** (ID: 002/200) ***/
void DISP_line2nd_menu_IDxxxSel(u8 _sel, u8 _total)
{
	OLED_ShowString12x24((1 << 3), 2, "ID:   /"); 	//"ID:   /"
	OLED_ShowNum12x24((1 << 3)+ 36, 2, _sel, 3, 24);
	OLED_ShowNum12x24((1 << 3)+ 36 + 48, 2, _total, 3, 24);
}

/***********************************************************/
paction_t_0 DISPTBL_L1_menulist[] = {	/** note: the order same as menuTab[] **/
	DISP_line2nd_finger,
	DISP_line2nd_mngpsword,
	DISP_line2nd_mngbluetooth,
	DISP_line2nd_system,
	DISP_line2nd_query,
};

void DISP_L1_menu(int _menuNO)
{
	DISPTBL_L1_menulist[_menuNO]();
}


void DISP_line2nd_FPSUCC(void)
{
	OLED_ShowString12x24(0, 2, "2.");   //2.
	OLED_ShowChinese24x24(2, 2, 55);	//添
	OLED_ShowChinese24x24(4, 2, 56);	//加
	OLED_ShowChinese24x24(6, 2, 10);	//用
	OLED_ShowChinese24x24(8, 2, 11);	//户
}

/***********************指纹管理****************************/
void DISP_menu_MNGFP_line1st(void)
{
	OLED_ShowChinese16x16(4, 0, 50);	  //指
	OLED_ShowChinese16x16(6, 0, 51);	  //纹
	OLED_ShowChinese16x16(8, 0, 27);	  //设
	OLED_ShowChinese16x16(10, 0, 28);	  //置
}

void DISP_menu_MNGFP_line2nd_addmngr(void)
{
	OLED_ShowString12x24((0 << 3), 2, "1");   //1.
	OLED_ShowString12x24((1 << 3), 2, ".");   //.
	
	OLED_ShowChinese24x24_Px_Py((2 << 3), 2, 55);		//添
	OLED_ShowChinese24x24_Px_Py((5 << 3) - 2, 2, 56);	//加
	OLED_ShowChinese24x24_Px_Py((8 << 3) - 4, 2, 67);	//管
	OLED_ShowChinese24x24_Px_Py((11 << 3) - 6, 2, 68);	//理
	OLED_ShowChinese24x24(13, 2, 69);	//员
}

void DISP_menu_MNGFP_line2nd_adduser(void)
{
	OLED_ShowString12x24((0 << 3), 2, "2.");   //2.
	OLED_ShowChinese24x24(3, 2, 55);		//添
	OLED_ShowChinese24x24(6, 2, 56);		//加
	OLED_ShowChinese24x24(9, 2, 57);		//用
	OLED_ShowChinese24x24(12, 2, 58);		//户
}

void DISP_menu_MNGFP_line2nd_delmngr(void)
{
	OLED_ShowString12x24((0 << 3), 2, "3");   	//3.
	OLED_ShowString12x24((1 << 3) + 1, 2, ".");   	//.
	
	OLED_ShowChinese24x24_Px_Py((2 << 3), 2, 53);		//删
	OLED_ShowChinese24x24_Px_Py((5 << 3) - 2, 2, 54);	//除
	OLED_ShowChinese24x24_Px_Py((8 << 3) - 4, 2, 67);	//管
	OLED_ShowChinese24x24_Px_Py((11 << 3) - 6, 2, 68);	//理
	OLED_ShowChinese24x24(13, 2, 69);	//员
}

void DISP_menu_MNGFP_line2nd_deluser(void)
{
	OLED_ShowString12x24(0, 2, "4.");   //4.
	OLED_ShowChinese24x24(3, 2, 53);	//删
	OLED_ShowChinese24x24(6, 2, 54);	//除
	OLED_ShowChinese24x24(9, 2, 57);	//用
	OLED_ShowChinese24x24(12, 2, 58);	//户
}

paction_t_0 DISPTBL_L2_fpmnglist[] = {
	DISP_menu_MNGFP_line2nd_addmngr,
	DISP_menu_MNGFP_line2nd_adduser,
	DISP_menu_MNGFP_line2nd_delmngr,
	DISP_menu_MNGFP_line2nd_deluser,
};

void DISP_L2_menu_fpmng(u8 _menuNO)
{
	DISPTBL_L2_fpmnglist[_menuNO]();
}

/***************************************/

void DISP_L3_menu_line1st_addmanager(void)
{
	OLED_ShowChinese16x16(3, 0, 6);		//添
	OLED_ShowChinese16x16(5, 0, 7);		//加
	OLED_ShowChinese16x16(7, 0, 10);	//管
	OLED_ShowChinese16x16(9, 0, 11);	//理
	OLED_ShowChinese16x16(11, 0, 12);	//员
}
void DISP_L3_menu_line1st_delmanager(void)
{
	OLED_ShowChinese16x16(3, 0, 8);		//删
	OLED_ShowChinese16x16(5, 0, 9);		//除
	OLED_ShowChinese16x16(7, 0, 10);	//管
	OLED_ShowChinese16x16(9, 0, 11);	//理
	OLED_ShowChinese16x16(11, 0, 12);	//员
}
/***************************************/

void DISP_L3_menu_line1st_adduser(void)
{
	OLED_ShowChinese16x16(4, 0, 6);		//添
	OLED_ShowChinese16x16(6, 0, 7);		//加
	OLED_ShowChinese16x16(8, 0, 13);	//用
	OLED_ShowChinese16x16(10, 0, 14);	//户
}
void DISP_L3_menu_line1st_deluser(void)
{
	OLED_ShowChinese16x16(4, 0, 8);		//删
	OLED_ShowChinese16x16(6, 0, 9);		//除
	OLED_ShowChinese16x16(8, 0, 13);	//用
	OLED_ShowChinese16x16(10, 0, 14);	//户
}

/** DISP_line2nd_menu_IDxxSel() **/
void DISP_menu_line2nd_PP(u8 __row)
{
	OLED_SetMatrixData(0, __row, 2, 3 ,0);			//clean
	OLED_SetMatrixData((13 << 3), __row, 3, 3 ,0);	//clean
	
	OLED_ShowChinese24x24(2, __row, 4);		//请
	OLED_ShowChinese24x24(5, __row, 5);		//按
	OLED_ShowChinese24x24(8, __row, 6);		//手
	OLED_ShowChinese24x24(11, __row, 7);	//指
}

/*****************************************************/
/** "管理员: 01" **/
void DISP_line2nd_PFadmin_ID( u8 _idx)
{
	OLED_ShowChinese24x24(1, 2, 67);			//管
	OLED_ShowChinese24x24(4, 2, 68);			//理
	OLED_ShowChinese24x24(7, 2, 69);			//员
	OLED_ShowString12x24((10 << 3), 2, ":"); 	//:
	
	OLED_ShowNum12x24((10 << 3) + 12, 2, _idx, 2, 24);	//02
}

/** "用户: 001" **/
void DISP_line2nd_PFuser_ID( u8 _idx)
{
	OLED_ShowChinese24x24(3, 2, 57);			//用
	OLED_ShowChinese24x24(6, 2, 58);			//户
	OLED_ShowString12x24((9 << 3), 2, ":"); 	//:
	
	OLED_ShowNum12x24((9 << 3) + 12, 2, _idx, 3, 24);	//003
}

/************************* 密码管理 ************************/
void DISP_menu_MNGPW_line1st(void)
{
	OLED_ShowChinese16x16(4, 0, 17);	  //密
	OLED_ShowChinese16x16(6, 0, 18);	  //码
	OLED_ShowChinese16x16(8, 0, 27);	  //设
	OLED_ShowChinese16x16(10, 0, 28);	  //置
}

void DISP_menu_MNGPW_line2nd_addPsword(void)
{
	OLED_ShowString12x24(0, 2, "1."); //1
	OLED_ShowChinese24x24(2, 2, 55);	//添
	OLED_ShowChinese24x24(5, 2, 56);	//加
	OLED_ShowChinese24x24(8, 2, 9);		//密
	OLED_ShowChinese24x24(11, 2, 10);	//码
	OLED_ShowString12x24((14 << 3), 2, " "); //" "
}

void DISP_menu_MNGPW_line2nd_delPsword(void)
{
	OLED_ShowString12x24(0, 2, "2."); //2.
	OLED_ShowChinese24x24(2, 2, 53);	//删
	OLED_ShowChinese24x24(5, 2, 54);	//除
	OLED_ShowChinese24x24(8, 2, 9);		//密
	OLED_ShowChinese24x24(11, 2, 10);	//码
}

void DISP_menu_MNGPW_line2nd_imgPsword(void)
{
	OLED_ShowString12x24(1, 2, "3.");   //3.
	OLED_ShowChinese24x24(2, 2, 82);	//虚
	OLED_ShowChinese24x24(5, 2, 83);	//位
	OLED_ShowChinese24x24(8, 2, 9);		//密
	OLED_ShowChinese24x24(11, 2, 10);	//码
}

paction_t_0 DISPTBL_L2_pwmnglist[] = {
	DISP_menu_MNGPW_line2nd_addPsword,
	DISP_menu_MNGPW_line2nd_delPsword,
	DISP_menu_MNGPW_line2nd_imgPsword,
};

void DISP_L2_menu_pwmng(int _menuNO)
{
	DISPTBL_L2_pwmnglist[_menuNO]();
}

/***********************添加/删除密码****************************/
void DISP_L3_menu_line1st_addPsword(void)
{
	OLED_ShowChinese16x16(4, 0, 6);		//添
	OLED_ShowChinese16x16(6, 0, 7);		//加
	OLED_ShowChinese16x16(8, 0, 17);	//密
	OLED_ShowChinese16x16(10, 0, 18);	//码
}

void DISP_L3_menu_line1st_delPsword(void)
{
	OLED_ShowChinese16x16(4, 0, 8);		//删
	OLED_ShowChinese16x16(6, 0, 9);		//除
	OLED_ShowChinese16x16(8, 0, 17);	//密
	OLED_ShowChinese16x16(10, 0, 18);	//码
}

void DISP_L3_menu_line1st_imgPsword(void)
{
	OLED_ShowChinese16x16(4, 0, 8);		//虚
	OLED_ShowChinese16x16(6, 0, 9);		//位
	OLED_ShowChinese16x16(8, 0, 17);	//密
	OLED_ShowChinese16x16(10, 0, 18);	//码
}

void DISP_L3_menu_line2nd_confirmDelPsword(void)
{
	OLED_SetMatrixData(0, 2, 2, 3 ,0);
	OLED_SetMatrixData((15 << 3), 2, 1, 3 ,0);
	
	OLED_ShowChinese24x24(2, 2, 51);	//确
	OLED_ShowChinese24x24(5, 2, 52);	//认
	OLED_ShowChinese24x24(8, 2, 53);	//删
	OLED_ShowChinese24x24(11, 2, 54);	//除
	
	OLED_ShowString12x24((14 << 3), 2, "?"); 	//？
}

void DISP_L3_menu_line2nd_inputPsword(void)
{
	OLED_ShowChinese24x24(1, 2, 59);	//输
	OLED_ShowChinese24x24(4, 2, 60);	//入
	OLED_ShowChinese24x24(7, 2, 9);		//密
	OLED_ShowChinese24x24(10, 2, 10);	//码
	
	OLED_ShowString12x24((13 << 3), 2, ":"); 	//:
}

/** "密码: 01" **/
void DISP_line2nd_PSword_ID(u8 _idx)
{
	OLED_ShowChinese24x24(3, 2, 9);				//密
	OLED_ShowChinese24x24(6, 2, 10);			//码
	OLED_ShowString12x24((9 << 3), 2, ":"); 	//:
	
	OLED_ShowNum12x24((9 << 3) + 12, 2, _idx, 2, 24);	//03
}

/** "添加成功" **/
void DISP_line3rd_AddSucc(void)
{
	OLED_ShowChinese24x24(2, 5, 55);	//添
	OLED_ShowChinese24x24(5, 5, 56);	//加
	OLED_ShowChinese24x24(8, 5, 33);	//成
	OLED_ShowChinese24x24(11, 5, 34);	//功
}

/** "添加失败" **/
void DISP_line3rd_AddFail(void)
{
	OLED_ShowChinese24x24(2, 3, 55);	//添
	OLED_ShowChinese24x24(5, 3, 56);	//加
	OLED_ShowChinese24x24(8, 3, 61);	//失
	OLED_ShowChinese24x24(11, 3, 62);	//败
}

/** "已满" **/
void DISP_line3rd_full(u8 __line)
{
	OLED_ShowChinese24x24(5, __line, 70);	//已
	OLED_ShowChinese24x24(8, __line, 73);	//满
}

/** "已清空" **/
void DISP_line3rd_empty(u8 __line)
{
	OLED_ShowChinese24x24(4, __line, 70);	//已
	OLED_ShowChinese24x24(7, __line, 71);	//清
	OLED_ShowChinese24x24(10, __line, 72);	//空
}
/*************************************************/
/** "正在删除" **/
void DISP_line3rd_delDoing(u8 __row)
{
	OLED_ShowChinese24x24(2, __row, 53);	//删
	OLED_ShowChinese24x24(5, __row, 54);	//除
	OLED_ShowChinese24x24(8, __row, 44);	//中
	OLED_ShowString12x24((11 << 3), __row, "...");   	//...
}

/** "删除成功" **/
void DISP_line3rd_delSucc(u8 __row)
{
	OLED_ShowChinese24x24(2, __row, 53);	//删
	OLED_ShowChinese24x24(5, __row, 54);	//除
	OLED_ShowChinese24x24(8, __row, 33);	//成
	OLED_ShowChinese24x24(11, __row, 34);	//功
}

/** "删除失败" **/
void DISP_line3rd_delFail(u8 __row)
{
	OLED_ShowChinese24x24(2, __row, 53);	//删
	OLED_ShowChinese24x24(5, __row, 54);	//除
	OLED_ShowChinese24x24(8, __row, 61);	//失
	OLED_ShowChinese24x24(11, __row, 62);	//败
}
/*************************************************/
void DISP_L3_menu_line2nd_ImgPsword(void)
{
	OLED_SetMatrixData(0, 2, 3, 3 ,0);	//clean
	OLED_ShowChinese24x24(2, 2, 82);	//虚
	OLED_ShowChinese24x24(5, 2, 83);	//位
	OLED_ShowChinese24x24(8, 2, 9);		//密
	OLED_ShowChinese24x24(11, 2, 10);	//码
}
/**********蓝牙设置*************************************************************/
void DISP_menu_MNGBT_line1st(void)
{
	OLED_ShowChinese16x16(4, 0, 65);	  //蓝
	OLED_ShowChinese16x16(6, 0, 66);	  //牙
	OLED_ShowChinese16x16(8, 0, 27);	  //设
	OLED_ShowChinese16x16(10, 0, 28);	  //置
}

void DISP_menu_MNGBT_line2nd_addBluetooth(void)
{
	OLED_ShowString12x24(0, 2, "1."); 	//1
	OLED_ShowChinese24x24(2, 2, 55);	//添
	OLED_ShowChinese24x24(5, 2, 56);	//加
	OLED_ShowChinese24x24(8, 2, 85);	//蓝
	OLED_ShowChinese24x24(11, 2, 86);	//牙
	OLED_ShowString12x24((14 << 3), 2, " "); //" "
}

void DISP_menu_MNGBT_line2nd_delBluetooth(void)
{
	OLED_ShowString12x24(0, 2, "2."); 	//2.
	OLED_ShowChinese24x24(2, 2, 53);	//删
	OLED_ShowChinese24x24(5, 2, 54);	//除
	OLED_ShowChinese24x24(8, 2, 85);	//蓝
	OLED_ShowChinese24x24(11, 2, 86);	//牙
}
/** 下一层 **/
paction_t_0 DISPTBL_L2_btmnglist[] = {
	DISP_menu_MNGBT_line2nd_addBluetooth,
	DISP_menu_MNGBT_line2nd_delBluetooth,
};

void DISP_L2_menu_btmng(u8 _menuNO)
{
	DISPTBL_L2_btmnglist[_menuNO]();
}
/** 再下一层 **/
void DISP_L3_menu_line1st_addBluetooth(void)
{
	OLED_ShowChinese16x16(4, 0, 6);		//添
	OLED_ShowChinese16x16(6, 0, 7);		//加
	OLED_ShowChinese16x16(8, 0, 65);	//蓝
	OLED_ShowChinese16x16(10, 0, 66);	//牙
}

void DISP_L3_menu_line1st_delBluetooth(void)
{
	OLED_ShowChinese16x16(4, 0, 8);		//删
	OLED_ShowChinese16x16(6, 0, 9);		//除
	OLED_ShowChinese16x16(8, 0, 65);	//蓝
	OLED_ShowChinese16x16(10, 0, 66);	//牙
}

/** "正在添加" **/
void DISP_line3rd_addDoing(u8 __row)
{
	OLED_ShowChinese24x24(2, __row, 55);	//添
	OLED_ShowChinese24x24(5, __row, 56);	//加
	OLED_ShowChinese24x24(8, __row, 44);	//中
	OLED_ShowString12x24((11 << 3), __row, "...");   	//...
}

/** "蓝牙: 01" **/
void DISP_line2nd_Bluetooth_ID(u8 _idx)
{
	OLED_ShowChinese24x24(3, 2, 85);			//蓝
	OLED_ShowChinese24x24(6, 2, 86);			//牙
	OLED_ShowString12x24((9 << 3), 2, ":"); 	//:
	
	OLED_ShowNum12x24((9 << 3) + 12, 2, _idx, 2, 24);	//03
}
/**********系统设置*************************************************************/
void DISP_menu_MNGSYS_line1st(void)
{
	OLED_ShowChinese16x16(4, 0, 25);	  //系
	OLED_ShowChinese16x16(6, 0, 26);	  //统
	OLED_ShowChinese16x16(8, 0, 27);	  //设
	OLED_ShowChinese16x16(10, 0, 28);	  //置
}

void DISP_menu_MNGSYS_line2nd_openMode(void)
{
	OLED_ShowString12x24(1, 2, "1.");   //1.
	OLED_ShowChinese24x24(3, 2, 25);	//开
	OLED_ShowChinese24x24(6, 2, 26);	//锁
	OLED_ShowChinese24x24(9, 2, 35);	//方
	OLED_ShowChinese24x24(12, 2, 36);	//式
}

void DISP_menu_MNGSYS_line2nd_SetDateTime(void)
{
	OLED_ShowString12x24(1, 2, "2.");   //2.
	OLED_ShowChinese24x24(3, 2, 13);	//设
	OLED_ShowChinese24x24(6, 2, 14);	//置
	OLED_ShowChinese24x24(9, 2, 19);	//时
	OLED_ShowChinese24x24(12, 2, 20);	//间
}

void DISP_menu_MNGSYS_line2nd_Restorefactory(void)
{
	OLED_ShowString12x24(1, 2, "3.");   //3.
	OLED_ShowChinese24x24(3, 2, 21);	//恢
	OLED_ShowChinese24x24(6, 2, 22);	//复
	OLED_ShowChinese24x24(9, 2, 23);	//出
	OLED_ShowChinese24x24(12, 2, 24);	//厂
}

paction_t_0 DISPTBL_L2_sysmnglist[] = {
	DISP_menu_MNGSYS_line2nd_openMode,
	DISP_menu_MNGSYS_line2nd_SetDateTime,
	DISP_menu_MNGSYS_line2nd_Restorefactory,
};

void DISP_L2_menu_sysmng(int _menuNO)
{
	DISPTBL_L2_sysmnglist[_menuNO]();
}

/***************************************/
void DISP_menu_MNGSYS_line1st_OpenMode(void)
{
	OLED_ShowChinese16x16(4, 0, 52);	  //开
	OLED_ShowChinese16x16(6, 0, 47);	  //锁
	OLED_ShowChinese16x16(8, 0, 63);	  //模
	OLED_ShowChinese16x16(10, 0, 64);	  //式
}

void DISP_menu_MNGSYS_line2nd_FPorPS(void)
{
	OLED_ShowChinese24x24_Px_Py((1 << 3), 2, 7);	//指
	OLED_ShowChinese24x24_Px_Py((4 << 3), 2, 8);	//纹
	OLED_ShowString12x24((7 << 3), 2, "/");   	//'/'
	OLED_ShowChinese24x24_Px_Py((10 << 3) - 12, 2, 9);	//密
	OLED_ShowChinese24x24_Px_Py((13 << 3) - 12, 2, 10);	//码
}

void DISP_menu_MNGSYS_line2nd_FPandPS(void)
{
	OLED_ShowChinese24x24_Px_Py((1 << 3), 2, 7);	//指
	OLED_ShowChinese24x24_Px_Py((4 << 3), 2, 8);	//纹
	OLED_ShowString12x24((7 << 3), 2, "+");   	//'+'
	OLED_ShowChinese24x24_Px_Py((10 << 3) - 12, 2, 9);	//密
	OLED_ShowChinese24x24_Px_Py((13 << 3) - 12, 2, 10);	//码
}

void DISP_menu_MNGSYS_line2nd_FPandFP(void)
{
	OLED_ShowChinese24x24_Px_Py((1 << 3), 2, 7);	//指
	OLED_ShowChinese24x24_Px_Py((4 << 3), 2, 8);	//纹
	OLED_ShowString12x24((7 << 3), 2, "+");   	//'+'
	OLED_ShowChinese24x24_Px_Py((10 << 3) - 12, 2, 7);	//指
	OLED_ShowChinese24x24_Px_Py((13 << 3) - 12, 2, 8);	//纹
}

paction_t_0 DISPTBL_L2_openModelist[] = {
	DISP_menu_MNGSYS_line2nd_FPorPS,
	DISP_menu_MNGSYS_line2nd_FPandPS,
	DISP_menu_MNGSYS_line2nd_FPandFP,
};

void DISP_L2_menu_OpenModemng(int _menuNO)
{
	DISPTBL_L2_openModelist[_menuNO]();
}

/***************************************/
void DISP_menu_MNGSYS_line2nd_RestoreConfirm(void)
{
	OLED_SetMatrixData((0 << 3), 2, 2, 3 ,0);	//clean
	OLED_SetMatrixData((15 << 3), 2, 2, 3 ,0);	//clean
	
	OLED_ShowChinese24x24(1, 2, 21);	//恢
	OLED_ShowChinese24x24(4, 2, 22);	//复
	OLED_ShowChinese24x24(7, 2, 23);	//出
	OLED_ShowChinese24x24(10, 2, 24);	//厂
	OLED_ShowString12x24((13 << 3), 2, "?");   //?
}
void DISP_menu_MNGSYS_line2nd_RestoreDoing(void)
{
	OLED_SetMatrixData((0 << 3), 2, 2, 3 ,0);	//clean
	OLED_ShowChinese24x24(2, 2, 21);	//恢
	OLED_ShowChinese24x24(5, 2, 22);	//复
	OLED_ShowChinese24x24(8, 2, 23);	//出
	OLED_ShowChinese24x24(11, 2, 24);	//厂
	OLED_SetMatrixData((14 << 3), 2, 2, 3 ,0);	//clean
}

void DISP_menu_MNGSYS_line3rd_Waiting(u8 __row)
{
	OLED_SetMatrixData((0 << 3),__row, 3, 2 ,0);	//clean
	OLED_SetMatrixData((11 << 3),__row, 5, 2 ,0);	//clean
	
	OLED_ShowChinese16x16(3, __row, 53);		//请
	OLED_ShowChinese16x16(5, __row, 54);		//稍
	OLED_ShowChinese16x16(7, __row, 37);		//等
	OLED_ShowString8x16((9 << 3), __row, "..."); 	//...
}

void DISP_menu_MNGSYS_line3rd_succ(u8 __row)
{
	OLED_SetMatrixData((0 << 3),__row, 5, 3 ,0);	//clean
	OLED_SetMatrixData((10 << 3),__row, 6, 3 ,0);	//clean
	
	OLED_ShowChinese24x24(4, __row, 33);	//成
	OLED_ShowChinese24x24(7, __row, 34);	//功
}

#if	0
void DISP_menu_MNGSYS_line3rd_over(u8 __row)
{
	OLED_SetMatrixData((0 << 3),__row, 5, 3 ,0);	//clean
	OLED_SetMatrixData((10 << 3),__row, 6, 3 ,0);	//clean
	
	OLED_ShowChinese24x24(4, __row, 84);	//完
	OLED_ShowChinese24x24(7, __row, 33);	//成
}
#endif

/***********************************************************
 * 第三行 功能已禁/启用
 ***********************************************************/
void DISP_line3rd_menu_enableStat(u8 __enable)
{
	OLED_SetMatrixData((0 << 3), 6, 5, 2 ,0);	//clean
	OLED_ShowChinese16x16(5, 6, 43);	  	//已
	
	if(__enable != 0) {
    	OLED_ShowChinese16x16(7, 6, 61);	//启
    } else {
		OLED_ShowChinese16x16(7, 6, 62);	//禁
	}
	
	OLED_ShowChinese16x16(9, 6, 13);	  	//用
	OLED_SetMatrixData((11 << 3), 6, 5, 2 ,0);			//clean
}


/*******************************************************************************
 * 无指纹模板时按set键显示
 *******************************************************************************/
void DISP_open_FPempty(void)
{
	OLED_ShowChinese24x24(0, 1, 4);		//请
	OLED_ShowChinese24x24(3, 1, 5);		//按
	OLED_ShowChinese24x24(6, 1, 13);	//设
	OLED_ShowChinese24x24(9, 1, 14);	//置
	OLED_ShowChinese24x24(12, 1, 74);	//键
	
	OLED_ShowChinese24x24(0, 4, 55);	//添
	OLED_ShowChinese24x24(3, 4, 56);	//加
	OLED_ShowChinese24x24(6, 4, 67);	//管
	OLED_ShowChinese24x24(9, 4, 68);	//理
	OLED_ShowChinese24x24(12, 4, 69);	//员
}

/*******************************************************************************
 * 上电显示
 *******************************************************************************/
void DISP_PowerOn(void)
{
	OLED_SetMatrixData((0 << 3),1, 2, 3 ,0);	//clean
	OLED_ShowChinese24x24(2, 1, 75);	//欢
	OLED_ShowChinese24x24(5, 1, 76);	//迎
	OLED_ShowChinese24x24(8, 1, 77);	//使
	OLED_ShowChinese24x24(11, 1, 57);	//用
	OLED_SetMatrixData((14 << 3),1, 2, 3 ,0);	//clean
	
	OLED_ShowChinese24x24(0, 4, 0);		//华
	OLED_ShowChinese24x24(3, 4, 1);		//园
	OLED_ShowChinese24x24(6, 4, 7);		//指
	OLED_ShowChinese24x24(9, 4, 8);		//纹
	OLED_ShowChinese24x24(12, 4, 26);	//锁
	OLED_SetMatrixData((15 << 3),4, 1, 3 ,0);	//clean
}

/*******************************************************************************
 * 低电校验/2nd line
 *******************************************************************************/
void DISP_lowPowerCheck(void)
{
	OLED_SetMatrixData((0 << 3),1, 2, 3 ,0);	//clean
	OLED_ShowChinese24x24(2, 1, 78);	//低
	OLED_ShowChinese24x24(5, 1, 79);	//电
	OLED_ShowChinese24x24(8, 1, 80);	//校
	OLED_ShowChinese24x24(11, 1, 63);	//验
	OLED_SetMatrixData((14 << 3),1, 2, 3 ,0);	//clean
}

/*******************************************************************************
 * 电压低，请更换电池
 *******************************************************************************/
void DISP_lowPower(void)
{
	OLED_SetMatrixData((0 << 3),0, 15, 1 ,0);	//上
	
	OLED_SetMatrixData((0 << 3),1, 3, 3 ,0);	//前
	OLED_ShowChinese24x24(3, 1, 79);	//电
	OLED_ShowChinese24x24(6, 1, 81);	//压
	OLED_ShowChinese24x24(9, 1, 78);	//低
	OLED_SetMatrixData((12 << 3),1, 4, 3 ,0);	//后
	
	OLED_SetMatrixData((0 << 3),4, 15, 1 ,0);	//中
	
	OLED_SetMatrixData((0 << 3), 5, 3, 2 ,0);	//前
	OLED_ShowChinese16x16(3, 5, 53);		//请
	OLED_ShowChinese16x16(5, 5, 55);		//更
	OLED_ShowChinese16x16(7, 5, 56);		//换
	OLED_ShowChinese16x16(9, 5, 57);		//电
	OLED_ShowChinese16x16(11, 5, 58);		//池	
	OLED_SetMatrixData((13 << 3), 5, 3, 2 ,0);	//后
	
	OLED_SetMatrixData((0 << 3), 7, 15, 1 ,0);	//下
}


/**********信息查询*************************************************************/
void DISP_menu_Query_line1st(void)
{
	OLED_ShowChinese16x16(4, 0, 33);	  //信
	OLED_ShowChinese16x16(6, 0, 34);	  //息
	OLED_ShowChinese16x16(8, 0, 35);	  //查
	OLED_ShowChinese16x16(10, 0, 36);	  //询
}

/*******************************************************************************
 * 版本号/时间
 *******************************************************************************/
 const unsigned char  version[32]= {
	#include "VERSION.h"
};

void DISP_version(void)
{
	OLED_ShowChinese16x16(0, 0, 48);		//版
	OLED_ShowChinese16x16(2, 0, 49);		//本
	OLED_ShowString8x16((4 << 3), 0, ": Ver"); 
	OLED_ShowString8x16((9 << 3), 0, (u8 *)version);
	
	OLED_ShowString8x16((0 << 3), 3, __DATE__);
	OLED_ShowString8x16((0 << 3), 6, __TIME__);
}

/*******************************************************************************
 * 当前时间显示(如果没有设置时间操作，则显示的是从上电到此时的运行总时间)
 * 格式如下：
 * 
 *******************************************************************************/
 u8 posTab_ShowNum8x16[][2] = {
	{6, 2}, 	//年
	{9, 2},		//月
	{12, 2},	//日
	{6, 4},		//时	
	{9, 4},		//分
	{12, 4},	//秒
};

void DISP_GetOrSetDateTime(u8 __setget)
{
	if(__setget == 0) {
		OLED_ShowChinese16x16(6, 0, 29);	//时
		OLED_ShowChinese16x16(8, 0, 30);	//间
	} else if(__setget == 1){
		OLED_ShowChinese16x16(6, 0, 27);	//设
		OLED_ShowChinese16x16(8, 0, 28);	//置
		OLED_ShowChinese16x16(6, 0, 29);	//时
		OLED_ShowChinese16x16(8, 0, 30);	//间
	} else {
		/** do nothing **/
	}
	OLED_ShowChinese16x16(0, 2, 31);	//日
	OLED_ShowChinese16x16(2, 2, 32);	//期
  //OLED_ShowString8x16((4 << 3), 2, ": 00-01-01");
	OLED_ShowString8x16((4 << 3), 2, ":   -  -  ");	//位置：6,7-年 9,10月 12,13日
	
	OLED_ShowChinese16x16(0, 4, 29);	//时
	OLED_ShowChinese16x16(2, 4, 30);	//间
  //OLED_ShowString8x16((4 << 3), 4, ": 00:00:00");
	OLED_ShowString8x16((4 << 3), 4, ":   :  :  ");	//位置：6,7-时 9,10分 12,13秒
/*************************************************/	
//Note: display the BCD code
	OLED_ShowNum8x16((6 << 3), 2, ((g_u8password[6] >> 4) & 0x0f), 1, 8);	//年
	OLED_ShowNum8x16((7 << 3), 2, (g_u8password[6] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((9 << 3), 2, ((g_u8password[5] >> 4) & 0x01), 1, 8);	//月
	OLED_ShowNum8x16((10 << 3), 2, (g_u8password[5] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((12 << 3), 2, ((g_u8password[3] >> 4) & 0x03), 1, 8);	//日
	OLED_ShowNum8x16((13 << 3), 2, (g_u8password[3] & 0x0f), 1, 8);	
	
	OLED_ShowNum8x16((6 << 3), 4, ((g_u8password[2] >> 4) & 0x03), 1, 8);	//时
	OLED_ShowNum8x16((7 << 3), 4, (g_u8password[2] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((9 << 3), 4, ((g_u8password[1] >> 4) & 0x07), 1, 8);	//分
	OLED_ShowNum8x16((10 << 3), 4, (g_u8password[1] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((12 << 3), 4, ((g_u8password[0] >> 4) & 0x07), 1, 8);	//秒
	OLED_ShowNum8x16((13 << 3), 4, (g_u8password[0] & 0x0f), 1, 8);	
}

/*******************************************************************************
 * 当前时间(如果没有设置时间操作，则显示的是从上电到此时的运行总时间)
 * 格式如下：
 * 
 *******************************************************************************/
 #if	0
void DISP_GetDateTime(void)
{
	DISP_GetOrSetDateTime(0);
}
#endif
/*******************************************************************************
 * 开锁记录
 * 
 *******************************************************************************/
void DISP_QueryRecord_line2nd(void)
{
	OLED_ShowString12x24(0, 2, "1."); 	//1.
	OLED_ShowChinese24x24(2, 2, 25);	//开
	OLED_ShowChinese24x24(5, 2, 26);	//锁
	OLED_ShowChinese24x24(8, 2, 27);	//记
	OLED_ShowChinese24x24(11, 2, 28);	//录
	OLED_ShowString12x24((14 << 3), 2, " "); //" "
}

void DISP_QueryDateTime_line2nd(void)
{
	OLED_ShowString12x24(0, 2, "2."); 	//2.
	OLED_ShowChinese24x24(2, 2, 89);	//日
	OLED_ShowChinese24x24(5, 2, 90);	//期
	OLED_ShowChinese24x24(8, 2, 19);	//时
	OLED_ShowChinese24x24(11, 2, 20);	//间
	OLED_ShowString12x24((14 << 3), 2, " "); //" "
}

void DISP_QueryVersion_line2nd(void)
{
	OLED_ShowString12x24(0, 2, "3."); 	//3.
	OLED_ShowChinese24x24(2, 2, 87);	//版
	OLED_ShowChinese24x24(5, 2, 88);	//本
	OLED_ShowChinese24x24(8, 2, 27);	//记
	OLED_ShowChinese24x24(11, 2, 28);	//录
	OLED_ShowString12x24((14 << 3), 2, " "); //" "
}

/*******************************************************************************
 * 显示指定存贮空间的日期/时间数据
 *
 *			   0    1   2   3   4    5   6    7   8    9    10  11  12 13
 * 数据项格式: 0xAA+len+sec+min+hour+day+week+mon+year+flag+FPL+FPH+PW+BT
 *
 *******************************************************************************/
//void DISP_DateTime(u8 *__datetime)
void DISP_DateTime(u8 *__record)
{

	OLED_ShowChinese16x16(0, 2, 31);	//日
	OLED_ShowChinese16x16(2, 2, 32);	//期
  //OLED_ShowString8x16((4 << 3), 2, ": 00-01-01");
	OLED_ShowString8x16((4 << 3), 2, ":   -  -  ");	//位置：6,7-年 9,10月 12,13日
	
	OLED_ShowChinese16x16(0, 4, 29);	//时
	OLED_ShowChinese16x16(2, 4, 30);	//间
  //OLED_ShowString8x16((4 << 3), 4, ": 00:00:00");
	OLED_ShowString8x16((4 << 3), 4, ":   :  :  ");	//位置：6,7-时 9,10分 12,13秒
/*************************************************/	
//Note: display the BCD code
	//OLED_ShowNum8x16((6 << 3), 2, ((__datetime[6] >> 4) & 0x0f), 1, 8);	//年
	#if	0
		OLED_ShowNum8x16((6 << 3), 2, ((g_u8password[8] >> 4) & 0x0f), 1, 8);	//年
		OLED_ShowNum8x16((7 << 3), 2, (g_u8password[8] & 0x0f), 1, 8);
		
		OLED_ShowNum8x16((9 << 3), 2, ((g_u8password[7] >> 4) & 0x01), 1, 8);	//月
		OLED_ShowNum8x16((10 << 3), 2, (g_u8password[7] & 0x0f), 1, 8);
		
		OLED_ShowNum8x16((12 << 3), 2, ((g_u8password[5] >> 4) & 0x03), 1, 8);	//日
		OLED_ShowNum8x16((13 << 3), 2, (g_u8password[5] & 0x0f), 1, 8);	
		
		OLED_ShowNum8x16((6 << 3), 4, ((g_u8password[4] >> 4) & 0x03), 1, 8);	//时
		OLED_ShowNum8x16((7 << 3), 4, (g_u8password[4] & 0x0f), 1, 8);
		
		OLED_ShowNum8x16((9 << 3), 4, ((g_u8password[3] >> 4) & 0x07), 1, 8);	//分
		OLED_ShowNum8x16((10 << 3), 4, (g_u8password[3] & 0x0f), 1, 8);
		
		OLED_ShowNum8x16((12 << 3), 4, ((g_u8password[2] >> 4) & 0x07), 1, 8);	//秒
		OLED_ShowNum8x16((13 << 3), 4, (g_u8password[2] & 0x0f), 1, 8);	
	#else
	OLED_ShowNum8x16((6 << 3), 2, ((__record[8] >> 4) & 0x0f), 1, 8);	//年
	OLED_ShowNum8x16((7 << 3), 2, (__record[8] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((9 << 3), 2, ((__record[7] >> 4) & 0x01), 1, 8);	//月
	OLED_ShowNum8x16((10 << 3), 2, (__record[7] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((12 << 3), 2, ((__record[5] >> 4) & 0x03), 1, 8);	//日
	OLED_ShowNum8x16((13 << 3), 2, (__record[5] & 0x0f), 1, 8);	
	
	OLED_ShowNum8x16((6 << 3), 4, ((__record[4] >> 4) & 0x03), 1, 8);	//时
	OLED_ShowNum8x16((7 << 3), 4, (__record[4] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((9 << 3), 4, ((__record[3] >> 4) & 0x07), 1, 8);	//分
	OLED_ShowNum8x16((10 << 3), 4, (__record[3] & 0x0f), 1, 8);
	
	OLED_ShowNum8x16((12 << 3), 4, ((__record[2] >> 4) & 0x07), 1, 8);	//秒
	OLED_ShowNum8x16((13 << 3), 4, (__record[2] & 0x0f), 1, 8);	
	#endif
}

/*******************************************************************************
 * 第一行可能出现以下4种可能的显示
 *|----------------|
 *| 001 管理员:02  |
 *|----------------|
 *|----------------|
 *| 001 用户:002   |
 *|----------------|
 *|----------------|
 *| 001  密码:02   |
 *|----------------|
 *|----------------|
 *| 001  蓝牙:02   |
 *|----------------|
 *|----------------|
 *| 001 F:002+P:03 |
 *|----------------|
 *|----------------|
 *| 001 F:002+F:003|
 *|----------------|
 *
 *			   0    1   2   3   4    5   6    7   8    9    10  11  12 13
 * 数据项格式: 0xA0+len+sec+min+hour+day+week+mon+year+flag+FPL+FPH+PW+BT
 *
 *******************************************************************************/
void DISP_RecordId(u16 __idx, u8 *__record)
{
	u16 u16tmp, u16id;
	OLED_ShowString8x16((4 << 3), 0, "   ");   //"   "
	OLED_ShowString8x16((13 << 3), 0, "   ");   //"   "
	OLED_ShowNum8x16((1 << 3), 0, __idx, 3, 16);	//id
	
	if((__record[9] & 0x0f) == 1) {
		u16tmp = (__record[10] | (__record[11] << 8));
		if(MisADMIN(u16tmp)) {
			/** admin record**/
			OLED_ShowChinese16x16(6, 0, 10);	//管
			OLED_ShowChinese16x16(8, 0, 11);	//理
			OLED_ShowChinese16x16(10, 0, 12);	//员
			OLED_ShowString8x16((12 << 3), 0, ":");   //:
			OLED_ShowNum8x16((13 << 3), 0, M2AdminID(u16tmp), 2, 16);	//id
		} else {
			/** user record**/
			OLED_ShowChinese16x16(6, 0, 13);	//用
			OLED_ShowChinese16x16(8, 0, 14);	//户
			OLED_ShowString8x16((10 << 3), 0, ":");   //:
			OLED_ShowNum8x16((11 << 3), 0, M2UserID(u16tmp), 3, 16);	//id
		}
	} else if((__record[9] & 0x0f) == 2) {
		/** passwd record**/
		OLED_ShowChinese16x16(7, 0, 17);	//密
		OLED_ShowChinese16x16(9, 0, 18);	//码
		OLED_ShowString8x16((11 << 3), 0, ":");   //:
		OLED_ShowNum8x16((12 << 3), 0, __record[12], 2, 16);	//id
	} else if((__record[9] & 0x0f) == 3) {
		/** Finger & passwd record**/
		u16tmp = (__record[10] | (__record[11] << 8));
		if(MisADMIN(u16tmp)) {
			OLED_ShowString8x16((5 << 3), 0, "A:  +P:");   //:
			OLED_ShowNum8x16((7 << 3), 0, M2AdminID(u16tmp), 2, 16);	//id
			OLED_ShowNum8x16((12 << 3), 0, __record[12], 2, 16);	//id
		} else {
			OLED_ShowString8x16((5 << 3), 0, "U:   +P:");   //:
			OLED_ShowNum8x16((7 << 3), 0, M2UserID(u16tmp), 3, 16);	//id
			OLED_ShowNum8x16((13 << 3), 0, __record[12], 2, 16);	//id
		}
	} else if((__record[9] & 0x0f) == 4) {
		/** BT record**/
		OLED_ShowChinese16x16(7, 0, 65);	//蓝
		OLED_ShowChinese16x16(9, 0, 66);	//牙
		OLED_ShowString8x16((11 << 3), 0, ":");   //:
		OLED_ShowNum8x16((12 << 3), 0, __record[13], 2, 16);	//id
	} else if((__record[9] & 0x0f) == 9) {
		/** FP+FP record**/
		u16id = (__record[10] | (__record[11] << 8));
		u16tmp = (__record[12] | (__record[13] << 8));
		if(MisADMIN(u16id)) {
			if(MisADMIN(u16tmp)) {
				OLED_ShowString8x16((5 << 3), 0, "A:  +A:");   //:
				OLED_ShowNum8x16((7 << 3), 0, M2AdminID(u16id), 2, 16);	//id
				OLED_ShowNum8x16((12 << 3), 0,M2AdminID(u16tmp), 2, 16);	//id
			} else {
				OLED_ShowString8x16((5 << 3), 0, "A:  +U:");   //:
				OLED_ShowNum8x16((7 << 3), 0, M2AdminID(u16id), 2, 16);	//id
				OLED_ShowNum8x16((12 << 3), 0,M2UserID(u16tmp), 3, 16);	//id
			}
		} else {
			if(MisADMIN(u16tmp)) {
				OLED_ShowString8x16((5 << 3), 0, "U:   +A:");   //:
				OLED_ShowNum8x16((7 << 3), 0, M2UserID(u16id), 3, 16);	//id
				OLED_ShowNum8x16((13 << 3), 0,M2AdminID(u16tmp), 3, 16);	//id
			} else {
				OLED_ShowString8x16((5 << 3), 0, "U:   +U:");   //:
				OLED_ShowNum8x16((7 << 3), 0, M2UserID(u16id), 3, 16);	//id
				OLED_ShowNum8x16((13 << 3), 0,M2UserID(u16tmp), 3, 16);	//id
			}
		}
	} else {
	}
}

void DISP_NoRecord(void)
{
	OLED_ShowString12x24((0 << 3), 2, " "); //" "
	OLED_ShowChinese24x24(1, 2, 91);	//无
	OLED_ShowChinese24x24(4, 2, 25);	//开
	OLED_ShowChinese24x24(7, 2, 26);	//锁
	OLED_ShowChinese24x24(10, 2, 27);	//记
	OLED_ShowChinese24x24(13, 2, 28);	//录
}

paction_t_0 DISPTBL_L2_mngQueryList[] = {
	DISP_QueryRecord_line2nd,
	DISP_QueryDateTime_line2nd,
	DISP_QueryVersion_line2nd,
};

void DISP_L2_menu_mngQuery(u8 _menuNO)
{
	DISPTBL_L2_mngQueryList[_menuNO]();
}

/*******************************************************************************
 * 设置时间时的闪烁
 * input: __order - 指示闪烁位(年、月、日、时、分、秒)
 *		  __invert - 亮、暗
 *		  __data - 显示的数字
 *******************************************************************************/
void DISP_SetDateTime_Blink(u8 __order, u8 __invert, u8 __data)
{
	pfunc_v_u8_u8_u32_u8_u8_t funcTab_ShowNum8x16[] = {
		OLED_ShowNum8x16,
		OLED_ShowNum8x16_invert,
	};

	(funcTab_ShowNum8x16[__invert])((posTab_ShowNum8x16[__order][0] << 3), 
											posTab_ShowNum8x16[__order][1], 
											((__data >> 4) & 0x0f), 
											1, 
											8);
	(funcTab_ShowNum8x16[__invert])(((posTab_ShowNum8x16[__order][0] + 1) << 3), 
											posTab_ShowNum8x16[__order][1], 
											(__data & 0x0f), 
											1, 
											8);
}
