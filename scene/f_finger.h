#ifndef __F_FINGER_H__
#define __F_FINGER_H__

int f_finger(unsigned *pMsg);
int f_fingerInit(unsigned *pMsg);
int f_fingerExit(unsigned *pMsg);
int f_fingerError(unsigned *pMsg);
int f_fingerSearch(unsigned *pMsg);
int f_fingerSearchSuccess(unsigned *pMsg);
int f_fingerSearchFail(unsigned *pMsg);

int f_fingerEmpty(unsigned *pMsg);

/***********************************************************/

#define	MFPid_PUSH(x) do{	\
		if(g_u16FPid[0] < 2) {	\
			g_u16FPid[0]++;	\
			g_u16FPid[g_u16FPid[0]] = x;	\
		}	\
	}while(0)
#define	MFPid_POP(x) do{	\
	if(g_u16FPid[0] > 0)g_u16FPid[0]--;	\
	}while(0)

#define	MFPid_TOP(x)	(g_u16FPid[g_u16FPid[0] - (x)])

#define	MFPid_GETid(x)	(g_u16FPid[1])
//#define	MFPid_GET2ndid(x)	(g_u16FPid[2])
#define	MFPid_GETLEN(x)	(g_u16FPid[0])
#define	MisFPid_NEQ(x)	(g_u16FPid[1] != g_u16FPid[2])
#define	MFPid_CLEAN(x)	do{g_u16FPid[0] = 0;}while(0)

/***********************************************************/
#endif
