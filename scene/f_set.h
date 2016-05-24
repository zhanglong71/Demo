#ifndef __F_SET_H__
#define __F_SET_H__

int f_set(unsigned *data);

int f_set_addSuperAdmin(unsigned *pMsg);
int f_set_addSuperAdminDoing(unsigned *pMsg);

//int f_mngQuery(unsigned *pMsg);
int f_set_funcSel(unsigned *pMsg);
int f_set_authenSucc(unsigned *pMsg);
int f_set_authenFail(unsigned *pMsg);

//int f_setEmpty(unsigned *pMsg);
int f_set_lowPowerCali(unsigned *pMsg);

int f_set_vop(unsigned *pMsg);
int f_set_longPress(unsigned *pMsg);

#endif
