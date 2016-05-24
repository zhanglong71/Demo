
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "stm8s_awu.h"
#include "stm8s_tim3.h"

#include "w25x16.h"
#include "l_queryRecord.h"

#pragma location="record"
__root __far const u8  record[CRECORD_MAXCNT][CRECORD_MAXLEN] = {
  {CENTRYFLAG_BUSY, 0, 0, 1, 0},
};
/*******************************************************************************
 * a.����һ��������ʧ������
 * b.�˴�������һ����16byte�������
 * c.��һ��(���0)������֮��(�����й�����)���������������(����������)
 * d.�������ʽ: 0xAA+len+sec+min+hour+day+week+mon+yy+bitMap+id1+id2+idPW+id
 * e.�������ʽ��0xAA+(lenL+lenH)+(firstL+firstH)+(lastL+lastH)������
 *       len(lenL+lenH)����¼�Ѵ�������Ŀ����
 *       first(firstL+firstH)����¼Ŀǰ�����Ŀ��õ����µ�һ��ĵ�ַ
 *       last(lastL+lastH)����¼Ŀǰ���������ϵ�һ��ĵ�ַ(�书����len���ظ��������ݲ���֮)
 *       
 * f.��ѯ��������first��first+len���first+len��first�����������ʾ
 * g.��Ӷ�����������д�뵽firstָ�����
 *       �ٵ���first/len��ֵ
 *       first++�����first�������Χ������first=1;
 *       len++�����len�ﵽ���ֵMAX��������Ϊlen=MAX
 *
 * h.��ʼ�����̣��ϵ���Ҫ�������Ч���𣿲���Ҫ
 *
 * �����ռ�ʹ��
 * g_u8password[] 
 * 
 * ��¼������Ŀ����999Ϊ����(��ʾ3λ������)
 * 
 * w25x16��ǰ4page(4KB/page * 4page = 16KB)
 *******************************************************************************/
u16	record_read(u16 __id)
{
	u8 i;
	for(i = 0; i < CRECORD_MAXLEN; i++)g_u8password[i] = record[__id][i];
    return  0;
}

u16	record_write(u16 __idx)
{
 	u8  i;
	if(__idx < CRECORD_MAXCNT) {
		FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
		for(i = 0; i < CRECORD_MAXLEN; i++) {
			FLASH_ProgramByte((u32)&record[__idx][i], g_u8password[i]);
		}
		FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
	}
    return  0;
}

/** ���4ҳ **/
u16	record_clean(void)
{
	u16	i;
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	
	FLASH_ProgramByte((u32)&record[0][0], CENTRYFLAG_BUSY);
	FLASH_ProgramByte((u32)&record[0][1], 0);
	FLASH_ProgramByte((u32)&record[0][2], 0);
	FLASH_ProgramByte((u32)&record[0][3], 1);
	FLASH_ProgramByte((u32)&record[0][4], 0);
	
	for(i = 0; i < CRECORD_MAXCNT; i++) {
		FLASH_ProgramByte((u32)&record[i][0], 0);
	}
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
    return  0;
}

#if	1
/*************************************************/
u8	recordIdxTab_isIdxTabValid(void)
{
	return	(record[0][0] == CENTRYFLAG_BUSY);
}

void	recordIdxTab_setIdxTabValid(void)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&record[0][0], CENTRYFLAG_BUSY);
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}
/*************************************************/
u16	recordIdxTab_getRecordCnt(void)	
{
	return	(record[0][1] | (record[0][2] << 8));
}

void	recordIdxTab_setRecordCnt(u16 __cnt)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&record[0][1], (__cnt & 0xff));
	FLASH_ProgramByte((u32)&record[0][2], ((__cnt >> 8) & 0xff));
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}
/*************************************************
 * ��һ�����õ�ID
 *************************************************/
u16	recordIdxTab_getFreeRecordId(void)	
{
	if(recordIdxTab_isIdxTabValid()) {
		return	(record[0][3] | (record[0][4] << 8));
	} else {
		return	1;
	}
}

/*************************************************
 * �����µ�һ�����õ���ID
 *************************************************/
u16	recordIdxTab_getBusyRecordId(void)
{
	u16 tmp;
	
	if(recordIdxTab_isIdxTabValid()) {
		tmp = (record[0][3] | (record[0][4] << 8));
		if(tmp == 1) tmp = 999; else tmp--;
	} else {
		tmp = 0;
	}
	
	return	tmp;
}

void	recordIdxTab_setFreeRecordId(u16 __Id)
{
	FLASH_Unlock(FLASH_MEMTYPE_PROG);	//!!!!!!!!!!!!!!!!!
	FLASH_ProgramByte((u32)&record[0][3], (__Id & 0xff));
	FLASH_ProgramByte((u32)&record[0][4], ((__Id >> 8) & 0xff));
	FLASH_Lock(FLASH_MEMTYPE_PROG);		//!!!!!!!!!!!!!!!!!
}

void	recordIdxTab_update(void)	
{
	u16 u16tmp;
	
	if(recordIdxTab_isIdxTabValid()) {
		u16tmp = recordIdxTab_getRecordCnt();
		u16tmp += 1; if(u16tmp >= CRECORD_MAXCNT)u16tmp = 999;
		recordIdxTab_setRecordCnt(u16tmp);
		
		u16tmp = recordIdxTab_getFreeRecordId();
		u16tmp += 1; if(u16tmp >= CRECORD_MAXCNT)u16tmp = 1;
		recordIdxTab_setFreeRecordId(u16tmp);
	} else {
		recordIdxTab_setIdxTabValid();
		recordIdxTab_setRecordCnt(1);
		recordIdxTab_setFreeRecordId(2);
		nop();
	}
}
/*************************************************/
#else
/******************************************************************************/
#define	MRECORD_getDataAddr(x)		(g_u8password + 2)
#define	MRECORD_getRecordAddr(x)	(g_u8password)

#define	MRECORD_isRecordValid(x)	(g_u8password[0] == CENTRYFLAG_BUSY)
#define	MRECORD_setRecordHead(x)	(g_u8password[0] = CENTRYFLAG_BUSY)
#define	MRECORD_setRecordLen(x)		(g_u8password[1] = 16)
#define	MRECORD_setRecordFlag(x)	do{g_u8password[9] = (x);}while(0)
#define	MRECORD_cleanRecordFlag(x)	do{MRECORD_setRecordFlag(0);}while(0)
#define	MRECORD_setRecordFPid(x)	do{\
										g_u8password[10] = ((x) & 0xff);\
										g_u8password[11] = (((x) >> 8) & 0xff);\
										g_u8password[9] |= (1);\
									}while(0)
#define	MRECORD_setRecordPWid(x)	do{\
										g_u8password[12] = (x);\
										g_u8password[9] |= (1 << 1);\
									}while(0)
#define	MRECORD_setRecordBTid(x)	do{\
										g_u8password[13] = (x);\
										g_u8password[9] |= (1 << 2);\
									}while(0)

//--------------------------------------------------------------------------------------------------
#define	MRECORD_getIdxTabAddr(x)	(g_u8password + 20)

/**
#define	MRECORD_isIdxTabValid(x)	(g_u8password[20] == CENTRYFLAG_BUSY)/#define	MRECORD_setIdxTabValid(x)	(g_u8password[20] = CENTRYFLAG_BUSY)

#define	MRECORD_getLEN(x)		(g_u8password[20 + 1] | (g_u8password[20 + 2] << 8))
#define	MRECORD_setLEN(x)		do{\
									g_u8password[20 + 1] = (0xff & (x));\
									g_u8password[20 + 2] = (0xff & ((x) >> 8));\
								}while(0)

#define	MRECORD_getFIRST(x)		(g_u8password[20 + 3] | (g_u8password[20 + 4] << 8))
#define	MRECORD_setFIRST(x)		do{\
									g_u8password[20 + 3] = (0xff & (x));\
									g_u8password[20 + 4] = (0xff & ((x) >> 8));\
								}while(0)
#define	MRECORD_getFreeId(x)	(MRECORD_getFIRST(x))
*/
/*******************************************************************************/
#endif