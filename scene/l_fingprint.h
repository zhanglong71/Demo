#ifndef __L_FINGPRINT_H__

/********************************************/
#define FP_POWER_CTR_PORT       GPIOG
#define FP_POWER_CTR_PIN        GPIO_PIN_5

#define FP_RESET_CTR_PORT       GPIOC
#define FP_RESET_CTR_PIN        GPIO_PIN_4

#define FP_WAKE_CHK_PORT       GPIOE
#define FP_WAKE_CHK_PIN        GPIO_PIN_4

#define	MFPOWER_ON()	(GPIO_WriteHigh(FP_POWER_CTR_PORT, FP_POWER_CTR_PIN))
#define	MFPOWER_OFF()	(GPIO_WriteLow(FP_POWER_CTR_PORT, FP_POWER_CTR_PIN))

#define	MFPRESET_H()	(GPIO_WriteHigh(FP_RESET_CTR_PORT, GPIO_PIN_4))
#define	MFPRESET_L()	(GPIO_WriteLow(FP_RESET_CTR_PORT, GPIO_PIN_4))

/** 虚位密码 **/
#define	MenableImgPswd(x)	do{imgPswdEnable_setValue(CENTRYFLAG_BUSY);}while(0)
#define	MdisableImgPswd(x)	do{imgPswdEnable_setValue(CENTRYFLAG_IDLE);}while(0)

/** 开锁模式 **/
#define	MOpenMode_FPorPS(x)	do{OpenMode_setValue(0);}while(0)
#define	MOpenMode_FPandPS(x)	do{OpenMode_setValue(1);}while(0)
#define	MOpenMode_FPandFP(x)	do{OpenMode_setValue(2);}while(0)

/********************************************/
void FPort_Init(void);
void flashCache_init(void);

//int fingerTab_empty(void);
int fingerTab_isEmpty(void);
int fingerTab_entryIsBusy(u16 __idx);
void fingprintCmd(u8 *cmd);

void fingerTab_clean(void);
//void fingerTab_fill(u8 Manager_ID, u8 value);

void FP_INIT(void);
void FP_Register(u16 ID1, u16 ID2, u8 stime); 
void FP_DeRegister(u16 ID1,u16 ID2);
void FP_Query(u16 ID1, u16 ID2);
void FP_DelAll(void);
#define	MFP_DelAll()	(FP_DelAll())

//u8 FP_GetRespData(charBuf_queue_t *src, u8 arr[]);
u8 FP_GetRespData(void *src, u8 arr[]);

u8 FP_RespChk(u8 arr[]);
u16 FP_RespGetResult(u8 arr[]);
u16 FP_RespGetType(u8 arr[]);
u16 FP_RespGetLen(u8 arr[]);
u16 FP_RespGetId(u8 arr[]);
u16 FP_RespGetIdAttach(u8 arr[]);

//void fingerTab_init(void);

u8 fingerTab_getEntry(u16 idx);
void fingerTab_setEntry(u16 idx, u8 value);
int fingerTab_getBusyEntryNum(void);
//void fingerTab_fill(int begin, int end, u8 value);
//u8 FP_AddByManager(u8 Manager_ID);
int FP_AddByManager(u8 Manager_ID, u8 stime);
u8 FP_DelByManager(u8 Manager_ID);

int resetOP_getValue(void);
void resetOP_setValue(u8 value);
unsigned int VOL_getRefValue(void);
void VOL_setRefValue(int value);

u8 imgPswdEnable_getValue(void);
void imgPswdEnable_setValue(u8 value);
int isMmgPswdEnable(void);

u8 OpenMode_getValue(void);
void OpenMode_setValue(u8 value);

int isOpenMode_FPorPS(void);
int isOpenMode_FPandPS(void);
int isOpenMode_FPandFP(void);

int psword_isFull(void);
int psword_inputConfirm(u8 _password[]);

//u8 psword_Query(u8 _password[]);
u8 psword_Query_00(u8 _password[]);
//u8 psword_Query_10(u8 _password[]);
//u8 psword_Query_01(u8 _password[]);
u8 psword_Query_11(u8 _password[]);

typedef u8 (*psword_Query_t)(u8 _password[]);

//int psword_add(u8 _password[]);
int psword_add(u8 _password[], u8 _idx);
int psword_delAll(void);
int psword_del(u8 _idx);
int psword_isBusy(u8 _idx);
int psword_getBusyEntryNum(void);
u8 password_geterrno(void);
void password_seterrno( u8 value);

int bluetoothEntry_isBusy(u8 _idx);
int bluetoothEntry_del(u8 _idx);
int bluetoothEntry_delAll(void);
int bluetoothEntry_add(u8 __bluetooth[], u8 _idx);
u8 bluetooth_Query(u8 __bluetooth[]);

u8 BT_GetRespData(u8 arr[]);
u8 BT_RespGetType(u8 arr[]);
u8 BT_RespGetLen(u8 arr[]);
u8 BT_RespChksum(u8 arr[]);

void BT_ATcommand(u8 _cmd[]);
void BT_test(void);

void get_ChipID(u8 ChipUniqueID[]);

#endif
/////////////////////////////////////////////////////

