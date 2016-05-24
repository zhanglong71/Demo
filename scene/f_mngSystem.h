#ifndef __F_MNGSYSTEM_H__
#define __F_MNGSYSTEM_H__

int f_mngSystem(unsigned *pMsg);
int f_mngRestoreFactory(unsigned *pMsg);
int f_mngRestoreFactoryDoing(unsigned *pMsg);
int f_mngRestoreFactoryDone(unsigned *pMsg);

int f_mngOpenMode(unsigned *pMsg);
int f_mngOpenModeDone(unsigned *pMsg);

int f_mngDatetime(unsigned *pMsg);
int f_mngDatetimeDone(unsigned *pMsg);

#endif

/////////////////////////////////////////////////////

