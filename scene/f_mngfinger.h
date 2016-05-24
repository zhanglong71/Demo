
#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 
#include "debug.h"

#ifndef	__F_MNGFINGER_H__
#define	__F_MNGFINGER_H__

int f_mngFinger(unsigned *pMsg);
int f_mngFinger_addmngr(unsigned *pMsg);
int f_mngFinger_adduser(unsigned *pMsg);
int f_mngFinger_delmngr(unsigned *pMsg);
int f_mngFinger_deluser(unsigned *pMsg);

int f_mngFinger_addmngrDoing(unsigned *pMsg);
int f_mngFinger_addmngrSucc(unsigned *pMsg);
int f_mngFinger_addmngrFail(unsigned *pMsg);

int f_mngFinger_delmngrConfirm(unsigned *pMsg);
int f_mngFinger_delmngrDoing(unsigned *pMsg);
int f_mngFinger_delmngrSucc(unsigned *pMsg);
int f_mngFinger_delmngrFail(unsigned *pMsg);

int f_mngFinger_empty(unsigned *pMsg);
int f_mngFinger_full(unsigned *pMsg);

#endif
/////////////////////////////////////////////////////
