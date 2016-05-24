
#ifndef	__F_MNGPSWORD_H__
#define	__F_MNGPSWORD_H__

int f_mngPsword(unsigned *pMsg);
int f_mngPsword_add(unsigned *pMsg);
int f_mngPsword_AddDoing(unsigned *pMsg);
int f_mngPsword_AddSucc(unsigned *pMsg);
int f_mngPsword_AddFail(unsigned *pMsg);
int f_mngPsword_del(unsigned *pMsg);
int f_mngPsword_delDoing(unsigned *pMsg);
int f_mngPsword_delConfirm(unsigned *pMsg);
int f_mngPsword_delSucc(unsigned *pMsg);
int f_mngPsword_empty(unsigned *pMsg);
int f_mngPsword_full(unsigned *pMsg);

int f_mngPsword_Img(unsigned *pMsg);
int f_mngPsword_ImgDone(unsigned *pMsg);

#define	MPSWORD_PUSH(x) do{	\
	g_u8password[0] = ((g_u8password[0] >= CPSWORD_MAXLEN - 2)? (CPSWORD_MAXLEN - 2):(g_u8password[0] + 1));	\
	g_u8password[g_u8password[0]] = x;	\
	}while(0)
#define	MPSWORD_POP(x) do{	\
	if(g_u8password[0] > 0)g_u8password[0]--;	\
	}while(0)

#define	MPSWORD_GETLEN(x)	(g_u8password[0])
#define	MPSWORD_CLEAN(x)	do{g_u8password[0] = 0;}while(0)

#define	MisLOWPOWERCLEAN(x) do{g_u8password[0] = 0;}while(0)	//清空
#define	MisLOWPOWERCHECK(x) ((g_u8password[0] == 2) && (g_u8password[1] == CKEY_3) && (g_u8password[2] == CKEY_5))	//低电校验

#define	MisRESTORE(x) 		((g_u8password[0] == 2) && (g_u8password[1] == CKEY_3) && (g_u8password[2] == CKEY_7))	//恢复出厂

#endif
/////////////////////////////////////////////////////

