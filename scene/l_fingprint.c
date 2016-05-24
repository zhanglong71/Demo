#include "stm8s_flash.h"
#include "string.h"
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"
#include "debug.h"

#include "arch.h"
#include "charQue.h"
#include "driver.h"	
#include "l_fingprint.h"

/*******************************************************************************
Note:
	InkSTM8S207.icf的最扣一行添加以下内容，否则会有链拉错误
place at address  0x18000       { ro section font8x16,
                                  ro section font16x16,
                                  ro section font12x24,
                                  ro section font24x24,
                                  ro section config,
                                  ro section password,
                                  ro section bluetooth,
                                  ro section at_cmd_tab,
                                  ro section fpBITMAP };
********************************************************************************/

/*******************************************************************************
 * config数据存贮规划如下：
 * 0..3 - MAGICCODE(暂用HUAR的ASCII码作为识别)
 * 4..7 - VOL电压ADC(低电)
 * 8 - recovery flag
 * 9 - ~0/0 - 启用/禁用虚位密码
 * 10 - 开锁模式(0/1/2 - 指纹/密码，指纹+密码，指纹+指纹)
 * 11 - reserved
 * 12..31 - reserved
 *******************************************************************************/
 #pragma location="config"
__root __far const u8  config[32]= {
	'H', 'U', 'A', 'R', 0,
};

/*******************************************************************************
 * 第0个表示密码的是否有效的信息
 * 第1..CPSWORD_MAXCNT - 1
 *
 * 对每一密码项作以下规划：(0..31)
 * 0 - 是否有密码(占位)
 * 1 - 密码长度
 * 2..31 - 密码数据
 *******************************************************************************/
#pragma location="password"
__root __far const u8  password[CPSWORD_MAXCNT + 1][CPSWORD_MAXLEN] = {
  {0},
};
/*******************************************************************************
 * 第0个表示蓝牙配对信息是否有效的信息
 * 第1..CBLUETOOTH_MAXCNT - 1
 *
 * 对每一配对信息项作以下规划：(0..31)
 * 0 - 是否有配对信息(占位)
 * 1 - 信息长度
 * 2..31 - 配对信息数据
 *******************************************************************************/
#pragma location="bluetooth"
__root __far const u8  bluetooth[CBLUETOOTH_MAXCNT + 1][CBLUETOOTH_MAXLEN] = {
  {0},
};

#pragma location="at_cmd_tab"
__root __far const u8 at_cmd_tab[][AT_CMD_MAXLEN] = {
  "AT+NAME=HuaRainTech\r\n\0",
  //"AT+NAME=HuaRainTech\r\n",
};
/*******************************************************************************
 * 指纹模板索引位图
 * 0xAy/~0xAy - 已用/空闲
 * 其中的y共有4个bit，预留作它用
 * 
 * 第m(m >= 0)个指纹模板的索引位图所在位置:  (m)
 *******************************************************************************/
#pragma location="fpBITMAP"
__root __far const u8  fpbitMap[CFINGER_MAXCNT]= {
  {0},
};

/**********************************************************************
 * 指纹控制端口。
 **********************************************************************/
void FPort_Init(void)
{
	GPIO_Init(FP_POWER_CTR_PORT, FP_POWER_CTR_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);    //供电控制. 输出 
  
	GPIO_Init(FP_RESET_CTR_PORT, FP_RESET_CTR_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);    //复位控制. 输出 
	
	GPIO_Init(FP_WAKE_CHK_PORT, FP_WAKE_CHK_PIN, GPIO_MODE_IN_PU_IT);	      	//FP_wakecheck. 输入
}

/**********************************************************************
 * name: fingerTab_empty 
 * description: check weather the fingerprint REPO empty
 **********************************************************************/
int fingerTab_isEmpty(void)
{
	return	(!((fpbitMap[1] & 0xF0) == CENTRYFLAG_BUSY));
	//return	((fpbitMap[1] & 0xF0) != CENTRYFLAG_BUSY);
	//return	((fpbitMap[0] & 0xF0) != CENTRYFLAG_BUSY);
}

u8 fingerTab_getEntry(u16 idx)
{
	return	fpbitMap[idx];
}

void fingerTab_setEntry(u16 __idx, u8 __value)
	
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&fpbitMap[__idx], __value);	
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

/*******************************************************************************
 * 判断指定的目录项是否空闲/忙
 *******************************************************************************/
int fingerTab_entryIsBusy(u16 __idx)
{
	return	((fpbitMap[__idx] & 0xF0) == CENTRYFLAG_BUSY);
}
/*******************************************************************************
 * 对索引表中的指定位置的数据赋值(空闲CENTRYFLAG_IDLE)
 *******************************************************************************/
void fingerTab_clean(void)
{
  u16 i;
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	//for(i = 0; (i < (CTOTALFINGER - 1)); i++) {
	for(i = 0; (i < (MUSER_ID_MAX - 1)); i++) {
		FLASH_ProgramByte((u32)&fpbitMap[i], CENTRYFLAG_IDLE);
	}
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

/*******************************************************************************
 * 取索引表中的已占用的项的数目
 *******************************************************************************/
int fingerTab_getBusyEntryNum(void)
{
	int i;
	int count = 0;
	
	//for(i = 0; i < CTOTALFINGER; i++) {
	for(i = 0; i < MUSER_ID_MAX; i++) {
		if((fpbitMap[i] & 0xF0) == CENTRYFLAG_BUSY) {
			count++;
		}
	}
	
	return	count;
}
/*******************************************************************************/
u8 g_CMD[24];	/** only for fingerPrint or bluetooth command **/
/*******************************************************************************
 * This command have no responsed
 * JP100-A无此命令，但有reset脚对电路复位. 可不使用
 *******************************************************************************/
//#define	MFPRESET_H()	(GPIO_WriteHigh(FP_RESET_CTR_PORT, GPIO_PIN_4))
//#define	MFPRESET_L()	(GPIO_WriteLow(FP_RESET_CTR_PORT, GPIO_PIN_4))

//void	fpReset_h(void)	{GPIO_WriteHigh(FP_RESET_CTR_PORT, GPIO_PIN_4);}
//void	fpReset_l(void)	{GPIO_WriteLow(FP_RESET_CTR_PORT, GPIO_PIN_4);}

void FP_INIT(void)
{
	#if	1
		u8 i;
		u16 TxSum = 0;
		struct charData_s charData;
		
		for(i = 0; i < 24; i++)g_CMD[i] = 0;
    	
		//prefix
		g_CMD[0] = 0x55;
		g_CMD[1] = 0xAA;
		//CMD
		g_CMD[2] = 0x30;
		g_CMD[3] = 0x01;
		//LEN
		//g_CMD[4] = 0x00;
		//g_CMD[5] = 0x00;
		//DATA(ID)
		//g_CMD[6] = ((ID1) & 0xff);
		//g_CMD[7] = ((ID1 >> 8) & 0xff);
		
		for(i = 0; i < 22; i++) {
			TxSum += g_CMD[i];
			
			charData.ucVal = g_CMD[i];
			charQueueIn_irq(&g_com1TxQue, &charData); 
		}
		
		g_CMD[22] = (TxSum & 0xff);
		g_CMD[23] = ((TxSum >> 8) & 0xff);
		for(i = 22; i < 24; i++) {
			charData.ucVal = g_CMD[i];
			charQueueIn_irq(&g_com1TxQue, &charData); 
		}

    #endif
}
/*******************************************************************************
 * function: FPRegister
 * Description:注册指纹(命令长度固定为24bytes, LEN指定DATA域中的有效数据长度)
 * input: 	ID1 - 指定的ID范围起始号
 * 			ID2 - 指定的ID范围结束号
 *			stime - 第几次设定
 * output:　no
 * return:  no
 *
 * other: 串口收到...表示注册成功
 *******************************************************************************/
void FP_Register(u16 ID1, u16 ID2, u8 stime)
{
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;

	//prefix
	g_CMD[0] = 0x55;
	g_CMD[1] = 0xAA;
	//CMD
	g_CMD[2] = 0x03;
	g_CMD[3] = 0x01;
	//LEN
	g_CMD[4] = 0x02;
	g_CMD[5] = 0x00;
	//DATA(ID)
	g_CMD[6] = ((ID1) & 0xff);
	g_CMD[7] = ((ID1 >> 8) & 0xff);
	
	for(i = 0; i < 22; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
	
	g_CMD[22] = (TxSum & 0xff);
	g_CMD[23] = ((TxSum >> 8) & 0xff);
	for(i = 22; i < 24; i++) {
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
}
/*******************************************************************************
 * function: FPDeRegister
 * Description:注销指定范围内的指纹模板
 * input: 	ID1 - 指定范围的起始ID号
 * 		 	ID2 - 指定范围的结束ID号
 * output:　no
 * return:  no
 *
 * other: 此操作之后一般会修改索引表
 *******************************************************************************/
void FP_DeRegister(u16 ID1, u16 ID2)
{
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;

	//prefix
	g_CMD[0] = 0x55;
	g_CMD[1] = 0xAA;
	//CMD
	g_CMD[2] = 0x05;
	g_CMD[3] = 0x01;
	//LEN
	g_CMD[4] = 0x02;
	g_CMD[5] = 0x00;
	//DATA(ID)
	g_CMD[6] = ((ID1) & 0xff);
	g_CMD[7] = ((ID1 >> 8) & 0xff);
	
	for(i = 0; i < 22; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);		//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
	
	g_CMD[22] = (TxSum & 0xff);
	g_CMD[23] = ((TxSum >> 8) & 0xff);
	for(i = 22; i < 24; i++) {
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);		//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
}
/*******************************************************************************
 * function: FPQuery
 * 查询全部指纹(0..200+)
 * 
 * input: no
 * output:
 * return: no
 * other: 从串口收到的数据为0x3a+0x03+0xaa+0x01则表示此次查询成功
 *******************************************************************************/
void FP_Query(u16 ID1, u16 ID2)
{
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;

	//prefix
	g_CMD[0] = 0x55;
	g_CMD[1] = 0xAA;
	//CMD
	g_CMD[2] = 0x02;
	g_CMD[3] = 0x01;
	//LEN
	g_CMD[4] = 0x00;
	g_CMD[5] = 0x00;
	//DATA(ID)
	//g_CMD[6] = ((ID1) & 0xff);
	//g_CMD[7] = ((ID1 >> 8) & 0xff);
	
	for(i = 0; i < 22; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);	//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
	
	g_CMD[22] = (TxSum & 0xff);
	g_CMD[23] = ((TxSum >> 8) & 0xff);
	for(i = 22; i < 24; i++) {
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);		//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
}

/*******************************************************************************
 * function: FP_DelAll
 * Description: 删除全部指纹
 * input: no
 * output: no
 * return: no
 * 		
 *******************************************************************************/
 #if	1
void FP_DelAll(void) //根据管理权限 1，
{
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;

	//prefix
	g_CMD[0] = 0x55;
	g_CMD[1] = 0xAA;
	//CMD
	g_CMD[2] = 0x06;
	g_CMD[3] = 0x01;
	//LEN
	g_CMD[4] = 0x00;
	g_CMD[5] = 0x00;
	//DATA(ID)
	//g_CMD[6] = ((ID1) & 0xff);
	//g_CMD[7] = ((ID1 >> 8) & 0xff);
	
	for(i = 0; i < 22; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);		//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
	
	g_CMD[22] = (TxSum & 0xff);
	g_CMD[23] = ((TxSum >> 8) & 0xff);
	for(i = 22; i < 24; i++) {
		charData.ucVal = g_CMD[i];
		//charQueueIn_irq(&g_com2TxQue, &charData);		//?????????????????????????
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
}
#endif

/*******************************************************************************
 * function: FP_DelAll
 * Description: 删除全部指纹
 * input: no
 * output: no
 * return: no
 * 		
 *******************************************************************************/

void FP_GetStatus(u16 ID1) //根据管理权限 1，
{
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;

	//prefix
	g_CMD[0] = 0x55;
	g_CMD[1] = 0xAA;
	//CMD
	g_CMD[2] = 0x08;
	g_CMD[3] = 0x01;
	//LEN
	g_CMD[4] = 0x02;
	g_CMD[5] = 0x00;
	//DATA(ID)
	g_CMD[6] = ((ID1) & 0xff);
	g_CMD[7] = ((ID1 >> 8) & 0xff);
	
	for(i = 0; i < 22; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
	
	g_CMD[22] = (TxSum & 0xff);
	g_CMD[23] = ((TxSum >> 8) & 0xff);
	for(i = 22; i < 24; i++) {
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com1TxQue, &charData); 
	}
}

 /******************************************************************************
  * function: FP_GetRespData
  * Description: get all of the received data from FIFO
  *
  * input: src - pointer of FIFO
  * output: arr - the data get from FIFO
  * return: <= 24 the length of data get from FIFO
  *
  * g_uart1buf[]
  *******************************************************************************/
//u8 FP_GetRespData(charBuf_queue_t *src, u8 arr[])
u8 FP_GetRespData(void *src, u8 arr[])
{
    u8 i = 0;
    u8 ucTmp;
    
    #if	1
    while(MFPACK_FIFO_GETLEN("the count of pending") != 0) {
    	MFPACK_FIFO_OUT(ucTmp);
    	if((ucTmp == 0xAA) && (MFPACK_FIFO_GETLEN("the count of pending") != 0)) {	/** 找到第一个0xaa **/
        	MFPACK_FIFO_OUT(ucTmp);
        	if(ucTmp == 0x55) {	/** 找到紧邻的0x55 **/
        		arr[0] = 0xAA;
        		arr[1] = 0x55;
        		i = 1;
        		break;
        	}
    	}
    }
    if((i - 1) == 0) {
    	while(MFPACK_FIFO_GETLEN("the count of pending") != 0) {
    		i++;
    		if(i < 24) {
    			MFPACK_FIFO_OUT(arr[i]);
    		}
    	}
    	return	i + 1;		/** 收到正常数据而结束 **/
	}
	#endif
	
    return	0;		/** 没有收到正常数据 **/
}

/** 查返回值类型,  **/
//int FP_RespGetType(u8 arr[]) 
u16 FP_RespGetType(u8 arr[]) 
{
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
    	return	(arr[2] | (arr[3] << 8));
	}
	return	0xffff;		/** 不可识别的命令类型 **/
}

/** 查返回值长度 **/
//int FP_RespGetLen(u8 arr[])
u16 FP_RespGetLen(u8 arr[])
{
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
    	return	(arr[4] | (arr[5] << 8));
	}
	return	0xffff;		/** 不可识别的数据长度 **/
}

/** 查返回值类型,  **/
//int FP_RespGetId(u8 arr[])
u16 FP_RespGetId(u8 arr[])
{
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
    	return	(arr[8] | (arr[9] << 8));
	}
	return	0xffff;	/** unrecognized id **/
}

/** 查返回值附属数据 **/
u16 FP_RespGetIdAttach(u8 arr[])
{
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
    	return	(arr[10] | (arr[11] << 8));
	}
	return	0xffff;	/** unrecognized id **/
}

/** 查处理结果 **/
//int FP_RespGetResult(u8 arr[])
u16 FP_RespGetResult(u8 arr[])
{
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
    	return	(arr[6] | (arr[7] << 8));
	}
	return	0xffff;		//action failed
}

/***********************************************************
 * function: FP_RespOk
 * check responsed: verify the chksum
 ***********************************************************/
//int FP_RespChk(u8 arr[])
u8 FP_RespChk(u8 arr[])
{
	u8 i;
	u16 TxSum = 0;
	
	if((arr[0] == 0xaa) && (arr[1] == 0x55)) {
		for(i = 0; i < 22; i++) {
			TxSum += arr[i];
		}
		
		if(((TxSum & 0xff) == arr[22]) && (((TxSum >> 8) & 0xff) == arr[23])) {
			return	CMD_ACK_OK; 	/** success **/
		}
	}
	
	return	CMD_ACK_SUM_ERR;		/** failed **/
}

/**********************************************************************
 * name: VOL_getRefValue
 * description: get ADCref value(default value 0)
 **********************************************************************/
unsigned int VOL_getRefValue(void)
{
	return	(config[4] | (config[5] << 8));
}
/**********************************************************************
 * name: VOL_setRefValue 
 * description: get ADCref value(default value 0)
 **********************************************************************/
void VOL_setRefValue(int value)
{	
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&config[4], (value & 0xff));		/** 此组没有密码 **/
	FLASH_ProgramByte((u32)&config[5], ((value >> 8) & 0xff));		/** 此组没有密码 **/
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}
/**********************************************************************
 * name: resetOP_setValue/resetOP_getValue
 * description: get reset operation control value(default value 0xff)
 **********************************************************************/
#if 1
int resetOP_getValue(void)
{
	return	config[8];
}

void resetOP_setValue(u8 value)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&config[8], value);
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}
#endif

/**********************************************************************
 * name: imgPswdEnable_setValue/imgPswdEnable_getValue
 * description: get imaginary password control value(default value 0)
 **********************************************************************/
u8 imgPswdEnable_getValue(void)
{
	return	config[9];
}

void imgPswdEnable_setValue(u8 value)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&config[9], value);
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

int isMmgPswdEnable(void)
{
	return	(config[9] == CENTRYFLAG_BUSY);
}

/**********************************************************************
 * name: imgPswdEnable_setValue/imgPswdEnable_getValue
 * description: get imaginary password control value(default value 0)
 **********************************************************************/
u8 OpenMode_getValue(void)
{
	return	config[10];
}

void OpenMode_setValue(u8 value)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&config[10], value);
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

int isOpenMode_FPorPS(void)
{
	return	(config[10] == 0);
}

int isOpenMode_FPandPS(void)
{
	return	(config[10] == 1);
}

int isOpenMode_FPandFP(void)
{
	return	(config[10] == 2);
}



/*********************************************************************
 *
 *
 *********************************************************************/

//#define	CENTRYFLAG_BUSY	0xa5

/**********************************************************************
 * name: psword_isFull 
 * description: check weather the password is full
 **********************************************************************/
int psword_isFull(void)
{
#if 0
	int	i;
	for(i = 0; i < CPSWORDCNT; i++) {
		if((password[_idx][0] & 0xf0) != CENTRYFLAG_BUSY) {
			return	0;		/** find a idle entry **/
		}
	}
#endif
	return	1;
}
/**********************************************************************
 * name: psword_isEmpty 
 * description: check weather the password is  empty
 **********************************************************************/
int psword_isEmpty(void)
{
#if 0
	int	i;
	for(i = 0; i < CPSWORDCNT; i++) {
		if((password[_idx][0] & 0xf0) == CENTRYFLAG_BUSY) {
			return	0;
		}
	}
#endif
	return	1;
}
/**********************************************************************
 * name: psword_inputConfirm 
 * description: check weather the password is same as the last one
 * 比较password[1..len]与password[len+1..2*len]是否相等比对长度与password[0]/2
 **********************************************************************/
int psword_inputConfirm(u8 __password[])
{
	u8	i;
	u8 len;
	
	if(__password == 0)return	0;	
	if((__password[0] & 0x01) || (__password[0] < 2)) return	0;		/** 前后两次输入的密码为奇数个或小于2个 **/
	
	len = (__password[0] >> 1);
	for(i = 1; i <= len; i++) {
		if(__password[i] != __password[i + len]) {
			return	0;
		}
	}
	return	1;
}

/**********************************************************************
 * name: password_Query 
 * description: Query weather the password is registered
 * 密码以len+data0+data1+..+datan的形式存放
 * 输入参数_password[]也以同样的格式存放
 * 故而要比较的长度为_password[0] + 1(长度+密码)
 * 无虚位密码，比较长度及后面的输入数字)
 **********************************************************************/
//u8 psword_Query(u8 _password[])
u8 psword_Query_00(u8 _password[])
{
	u8 i, j;
	
	for(i = 1; i < CPSWORDCNT; i++)	{	//从第1项长始，第0项保留
		if((password[i][0] & 0xf0) == CENTRYFLAG_BUSY) {	/** 此组有密码, 可以进行比较 **/
			for(j = 0; ((j < CPSWORDLEN - 2) && (j < _password[0] + 1)); j++) {
				if(password[i][j + 1] != _password[j]) {
					break;				/** 不相等，当前一组密码比对失败，退出并比较下一组 **/
				}
			}
			if(j == _password[0] + 1) {	/** 全部比较后退出，也就是比较通过 **/
				return	i;				/** 查找成功 **/
			}
		}
	}
    
	return	0;	/** 查找失败 **/
}

#if	0
/**********************************************************************
 * name: psword_Query_10 
 * description: Query weather the password is registered
 * 密码以len+data0+data1+..+datan的形式存放
 * 输入参数_password[]也以同样的格式存放
 * 故而要比较的长度为password[i][1](仅密码，无长度)
 * 
 * 带有防窥视功能(前面有虚位密码，只比较后面的输入数字)
 **********************************************************************/
u8 psword_Query_10(u8 _password[])
{
	u8 i, j;
	
	for(i = 1; i < CPSWORDCNT; i++)	{	//从第1项长始，第0项保留
		if(((password[i][0] & 0xf0) == CENTRYFLAG_BUSY) && (_password[0] >= password[i][1])) {	/** 此组有密码，并且输入长度大于或等于即有长度, 可以进行比较 **/
			for(j = password[i][1]; j > 0; j--) {
				if(password[i][j + 1] != _password[j]) {
					break;				/** 不相等，当前一组密码比对失败，退出并比较下一组 **/
				}
			}
			if(j == 0) {	/** 全部比较后退出，也就是比较通过 **/
				return	i;			/** 查找成功 **/
			}
		}
	}
    
	return	0;	/** 查找失败 **/
}

/**********************************************************************
 * name: psword_Query_01 
 * 比较的长度为password[i][1](仅密码，无长度)
 * 
 * 带有防窥视功能(后面有虚位密码，只比较前面的输入数字)
 **********************************************************************/
u8 psword_Query_01(u8 _password[])
{
	u8 i, j;
	
	for(i = 1; i < CPSWORDCNT; i++)	{	//从第1项长始，第0项保留
		if(((password[i][0] & 0xf0) == CENTRYFLAG_BUSY) && (_password[0] >= password[i][1])) {	/** 此组有密码，并且输入长度大于或等于即有长度, 可以进行比较 **/
			for(j = 0; j < password[i][1]; j++) {
				if(password[i][j + 2] != _password[j + 1]) {
					break;				/** 不相等，当前一组密码比对失败，退出并比较下一组 **/
				}
			}
			if(j >= password[i][1]) {	/** 全部比较后退出，也就是比较通过 **/
				return	i;			/** 查找成功 **/
			}
		}
	}
    
	return	0;	/** 查找失败 **/
}
#endif

/**********************************************************************
 * name: psword_Query_11 
 * 比较的长度为password[i][1](仅密码，无长度)
 * 
 * 带有防窥视功能(前后都有虚位密码，进行模式匹配)
 **********************************************************************/
u8 psword_Query_11(u8 _password[])
{
	u8 i, j, k;
	
	for(i = 1; i < CPSWORDCNT; i++)	{	//从第1项长始，第0项保留
		k = _password[0] - password[i][1] + 1;	//密码长度差+1，是可允许的模式匹配次数
		while(((password[i][0] & 0xf0) == CENTRYFLAG_BUSY) && (k > 0)) {	/** 此组有密码，并且输入长度大于或等于即有长度, 可以进行比较 **/
			for(j = 0; j < password[i][1]; j++) {	//逐位比较
				//if(password[i][j + 2] != _password[j + 1 + k - 1]) {
				if(password[i][j + 2] != _password[j + k]) {
					break;				/** 不相等，当前一组密码比对失败，退出并比较下一组 **/
				}
			}
			if(j >= password[i][1]) {	/** 全部比较后退出，也就是比较通过 **/
				return	i;				/** 查找成功 **/
			}
			
			k--;
		}
	}
    
	return	0;	/** 查找失败 **/
}


 /******************************************************************************
  *	function: 
  * Description: 添加一组密码 
  * input: 	__password[]
  *			idx
  * 	输入的密码是以2倍的长度及2组相同的串
  * output: 
  * return:  成功返回1, 失败返回0
  *
  * other: 注意密码的存贮格式
  * data:	0	1	2	3	4	...
  *			F	len	p0	p1	p2	...	p(len-1)
  ******************************************************************************/
/***指定 idx 添加一组密码***/
int psword_add(u8 __password[], u8 _idx)
{
	u8 i;
	
	if(_idx >= CPSWORDCNT) {
		return	0;
	}
	
	__password[0] >>= 1;
	if(password[_idx][0] != CENTRYFLAG_BUSY)	{	/** 确认此组没有密码, 可以存入 **/
		FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
		FLASH_ProgramByte((u32)&password[_idx][0], CENTRYFLAG_BUSY);	
		for(i = 0; ((i < CPSWORDLEN - 2) && (i < __password[0] + 1)); i++) {
			FLASH_ProgramByte((u32)&password[_idx][i + 1], __password[i]);
		}
		FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
		return	1;		/** 添加成功 **/
	}
	
	return	0;	/** 添加失败 **/
}
/*******************************************************************************
 * function:
 * Description: 删除全部密码(对应索引的位置清零即可)
 * input: no
 * output: no
 * return: no
 *
 *******************************************************************************/
int psword_delAll(void)
{
	int i;
	
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	for(i = 0; i < CPSWORDCNT; i++)	FLASH_ProgramByte((u32)&password[i][0], CENTRYFLAG_IDLE);	/** 此组没有密码 **/
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
	
	return	1;
}

int psword_del(u8 _idx)
{	
	if(_idx >= CPSWORDCNT) {
		return	0;
	}
	
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&password[_idx][0], CENTRYFLAG_IDLE);		/** 此组没有密码 **/
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
	
	return	1;
}

int psword_isBusy(u8 _idx)
{	
	if(_idx >= CPSWORDCNT) {
		return	0;
	}
	
	return	((password[_idx][0] & 0xf0) == CENTRYFLAG_BUSY);
}

/*******************************************************************************
 * 取索引表中的已占用的项的数目
 *******************************************************************************/
int psword_getBusyEntryNum(void)
{
	int i;
	int count = 0;
	
	for(i = 1; i < CPSWORDCNT; i++) {
		if((password[i][0] & 0xf0) == CENTRYFLAG_BUSY) {
			count++;
		}
	}
	
	return	count;
}
/*******************************************************************************
 * get/set password_errno
 *******************************************************************************/
u8 password_geterrno(void)
{
	//return	FLASH_ReadByte(CFLASH_PASSWD_ERR);
	return	password[0][5];
}

void password_seterrno(u8 value)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&(password[0][CFLASH_PASSWD_ERR]) , value);
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

/*** 以下是蓝牙部分管理接口*****************************************************/

/**
  * @brief  check weather the entry is busy or not.
  * @param  _idx: The index of the bluetooth entry.
  * @retval the busyFlag is busy or not
  */
int bluetoothEntry_isBusy(u8 _idx)
{	
	if(_idx >= CBLUETOOTHCNT) {
		return	0;
	}
	
	return	((bluetooth[_idx][0] & 0xf0) == CENTRYFLAG_BUSY);
}

 /**
  * @brief  delete the specified entry.
  * @param  _idx: The index of the bluetooth entry.
  * @retval delete success or not
  */
int bluetoothEntry_del(u8 _idx)
{	
	if(_idx >= CBLUETOOTHCNT) {
		return	0;
	}
	
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&bluetooth[_idx][0], CENTRYFLAG_IDLE);		/** 此组没有密码 **/
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
	
	return	1;
}

/**
  * @brief  delete all entry.
  * @param  none
  * @retval 1
  */
int bluetoothEntry_delAll(void)
{
	int i;
	
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	for(i = 0; i < CBLUETOOTHCNT; i++) FLASH_ProgramByte((u32)&bluetooth[i][0], CENTRYFLAG_IDLE);	/** 此组没有密码 **/
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
	
	return	1;
}

 /**
  * @brief  add the specified entry.
  * @param  _idx: The index of the bluetooth entry.
  * @retval add success or not
  */
int bluetoothEntry_add(u8 __bluetooth[], u8 _idx)
{
	u8 i;
	
	if(_idx >= CBLUETOOTHCNT) {
		return	0;
	}
	
	if(bluetooth[_idx][0] != CENTRYFLAG_BUSY)	{	/** 确认此组空闲, 可以存入 **/
		FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
		FLASH_ProgramByte((u32)&bluetooth[_idx][0], CENTRYFLAG_BUSY);	
		for(i = 0; ((i < CBLUETOOTHLEN - 2) && (i < __bluetooth[0] + 1)); i++) {
			FLASH_ProgramByte((u32)&bluetooth[_idx][i + 1], __bluetooth[i]);
		}
		FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
		return	1;		/** 添加成功 **/
	}
	
	return	0;	/** 添加失败 **/
}

 /**
  * @brief  query id info with specified bluetooth info.
  * @param  __bluetooth: the bluetooth info.
  * @retval query result: success or not
  */
u8 bluetooth_Query(u8 __bluetooth[])
{
	u8 i, j;
	
	for(i = 1; i < CBLUETOOTHCNT; i++)	{	//从第1项长始，第0项保留
		if((bluetooth[i][0] & 0xf0) == CENTRYFLAG_BUSY) {	/** 此组有数据有效, 可以进行比较 **/
			for(j = 0; ((j < CBLUETOOTHLEN - 2) && (j < __bluetooth[0] + 1)); j++) {
				if(bluetooth[i][j + 1] != __bluetooth[j]) {
					break;				/** 不相等，当前一组信息比对失败，退出当前组的比较，比较下一组 **/
				}
			}
			if(j == __bluetooth[0] + 1) {	/** 全部比较后退出，也就是比较通过 **/
				return	i;				/** 查找成功 **/
			}
		}
	}
    
	return	0;	/** 查找失败 **/
}

/******************************************************************************
  * function: BT_GetRespData
  * Description: get all of the received data from FIFO
  *
  * input: src - pointer of FIFO
  * output: arr - the data get from FIFO
  * return: <= 24 the length of data get from FIFO
  *
  * g_uart3buf[]
  * 
  * identify：	0x5A+0x5A+len+B1+B2+...+B15+B(chksum1)+B(chksum2)
  * enroll：	0x5A+0xA5+len+B1+B2+...+B15+B(chksum1)+B(chksum2)
  *     使用IMEI/devId表示的B1+B2+...+B15由数字组成可避免与0x5A/0xA5冲突
  *
  * AT command response: OK:xxxxx
  *******************************************************************************/
u8 BT_GetRespData(u8 __arr[])
{
    u8 i = 0;
    u8 ucTmp;
    
    while(MBTACK_FIFO_GETLEN("the count of pending") != 0) {
    	MBTACK_FIFO_OUT(ucTmp);
    	if((ucTmp == 'O') && (MBTACK_FIFO_GETLEN("the count of pending") != 0)) {
    		MBTACK_FIFO_OUT(ucTmp);
    		if((ucTmp == 'K') && (MBTACK_FIFO_GETLEN("the count of pending") != 0)) {
    			MBTACK_FIFO_OUT(ucTmp);
    			if((ucTmp == ':') && (MBTACK_FIFO_GETLEN("the count of pending") != 0)) {
    				__arr[0] = 'O';
    				__arr[1] = 'K';
    				__arr[2] = ':';
    				i = 3;
    			}
    		}
    	} else if((ucTmp == CCMD_BTHEAD) && (MBTACK_FIFO_GETLEN("the count of pending") != 0)) {	/** 找到第一个0x5a **/
        	__arr[0] = CCMD_BTHEAD;
        	i = 1;
        	break;
    	}
    }
    if(i != 0) {
    	while(MBTACK_FIFO_GETLEN("the count of pending") != 0) {
    		if(i < 32) {
    			MBTACK_FIFO_OUT(__arr[i]);
    		}
    		i++;
    	}
    	return	i + 1;		/** 收到正常数据而结束 **/
	}
	
    return	0;			/** 没有收到正常数据 **/
}

/** 查返回值类型 **/
u8 BT_RespGetType(u8 arr[])
{
    return	arr[1];		/** 命令类型 **/
}

/** 查返回值长度 **/
u8 BT_RespGetLen(u8 arr[])
{
	return	arr[2];		/** 命令长度 **/
}

/***********************************************************
 * function: BT_RespChksum
 * check responsed: verify the chksum
 ***********************************************************/
u8 BT_RespChksum(u8 arr[])
{
	u8 i;
	u16 TxSum = 0;
	
	if((arr[0] == CCMD_BTHEAD) && 
		((arr[1] == CCMD_MATCH) || (arr[1] == CCMD_ENROLL))) {
		TxSum = arr[0] + arr[1];
		for(i = 0; ((i < arr[2]) && (i < 30)); i++) {
			TxSum += arr[i];
		}
		
		if(((TxSum & 0xff) == arr[arr[2] + 3]) && (((TxSum >> 8) & 0xff) == arr[arr[2] + 4])) {
			return	CMD_ACK_OK; 	/** success **/
		}
	}
	
	return	CMD_ACK_SUM_ERR;		/** failed **/
}

void BT_ATcommand(u8 _cmd[])
{
	u8 i;
	struct charData_s charData;
	
	for(i = 0; ((_cmd[i] != '\0') && (i < AT_CMD_MAXLEN)); i++) {
		charData.ucVal = _cmd[i];
		charQueueIn_irq(&g_com3TxQue, &charData); 
	}
}

void BT_test(void)
{
	#if	0
	u8 i;
	u16 TxSum = 0;
	struct charData_s charData;
	
	for(i = 0; i < 24; i++)g_CMD[i] = 0;
    
	//prefix
	g_CMD[0] = CCMD_BTHEAD;
	//response
	g_CMD[1] = 0xA5;
	//LEN
	g_CMD[2] = 0x15;
	//data
	for(i = 0; i < 15; i++) g_CMD[i + 3] = i + 0x11;
	
	for(i = 0; i < 18; i++) {
		TxSum += g_CMD[i];
		
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com3TxQue, &charData); 
	}
	
	g_CMD[18] = (TxSum & 0xff);
	g_CMD[19] = ((TxSum >> 8) & 0xff);
	for(i = 17; i < 18; i++) {
		charData.ucVal = g_CMD[i];
		charQueueIn_irq(&g_com3TxQue, &charData); 
	}
	#endif
}


/**********************************************************************
 * 获取唯一码
 **********************************************************************/
void get_ChipID(u8 ChipUniqueID[])
{
	ChipUniqueID[0] = *(__IO u16 *)(0x48CD);
	ChipUniqueID[1] = *(__IO u16 *)(0x48CE);
	ChipUniqueID[2] = *(__IO u16 *)(0x48CF);
	ChipUniqueID[3] = *(__IO u16 *)(0x48D0);
	ChipUniqueID[4] = *(__IO u16 *)(0x48D1);
	ChipUniqueID[5] = *(__IO u16 *)(0x48D2);
	ChipUniqueID[6] = *(__IO u16 *)(0x48D3);
	ChipUniqueID[7] = *(__IO u16 *)(0x48D4);
	ChipUniqueID[8] = *(__IO u16 *)(0x48D5);
	ChipUniqueID[9] = *(__IO u16 *)(0x48D6);
	ChipUniqueID[10] = *(__IO u16 *)(0x48D7);
	ChipUniqueID[11] = *(__IO u16 *)(0x48D8);
}