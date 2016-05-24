
#ifndef	__L_QUERYRECORD_H__
#define	__L_QUERYRECORD_H__

u16	record_read(u16 __id);
u16	record_write(u16 __id);
u16	record_clean(void);

//u16	recordTab_update(void);

u8	recordIdxTab_isIdxTabValid(void);
void	recordIdxTab_setIdxTabValid(void);
void	recordIdxTab_setRecordCnt(u16 __cnt);
u16	recordIdxTab_getRecordCnt(void);
u16	recordIdxTab_getFreeRecordId(void);
void	recordIdxTab_setFreeRecordId(u16 __Id);
u16	recordIdxTab_getBusyRecordId(void);
void	recordIdxTab_update(void);

/*******************************************************************************/
#define	MRECORD_getDataAddr(x)		(g_u8password + 2)
#define	MRECORD_getRecordAddr(x)	(g_u8password)

//#define	MRECORD_isRecordValid(x)	(g_u8password[0] == CENTRYFLAG_BUSY)
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
/** 在指纹+指纹的场景下使用 **/
#define	MRECORD_setRecord2ndFPid(x)	do{\
										g_u8password[12] = ((x) & 0xff);\
										g_u8password[13] = (((x) >> 8) & 0xff);\
										g_u8password[9] |= (1 << 3);\
									}while(0)
//--------------------------------------------------------------------------------------------------


#endif
