
#ifndef	__F_MNGBLUETOOTH_H__
#define	__F_MNGBLUETOOTH_H__

int f_mngBluetooth(unsigned *pMsg);
int f_mngBluetooth_add(unsigned *pMsg);
int f_mngBluetooth_del(unsigned *pMsg);

int f_mngBluetooth_empty(unsigned *pMsg);
int f_mngBluetooth_full(unsigned *pMsg);

int f_mngBluetooth_AddDoing(unsigned *pMsg);
int f_mngBluetooth_AddFail(unsigned *pMsg);
int f_mngBluetooth_AddSucc(unsigned *pMsg);

int f_mngBluetooth_delConfirm(unsigned *pMsg);
int f_mngBluetooth_delDoing(unsigned *pMsg);
int f_mngBluetooth_delSucc(unsigned *pMsg);

#endif
/////////////////////////////////////////////////////
