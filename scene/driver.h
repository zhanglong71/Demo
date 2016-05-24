#ifndef __DRIVER_H__
#define __DRIVER_H__

void adcSample_Start(adcData_t *adcData);
void adcSample_Stop(adcData_t *adcData);

void blinkInit(void);
void actionInit(void);
void promptInit(void);
void actionDoing(actionQueue_t * q);
void actProcess(actionQueue_t * q);
void stopAction(actionQueue_t *q);

int value_inc(int _value, int _max, int _min);
int value_dec(int _value, int _max, int _min);

void BCD2HEX_bytes(u8 _bcd[], u8 len);
void HEX2BCD_bytes(u8 _hex[], u8 len);

void DAEMON_USART1_Send(charBuf_queue_t* comTxQue);
void DAEMON_USART3_Send(charBuf_queue_t* comTxQue);
//void DAEMON_USART1_Recive(charBuf_queue_t* comRxQue);

void ClkInit(void);
void Halt_OffDevice(void);
void Halt_OnDevice(void);
void Halt_OnAWU(void);
void Peripheral_Init(void);

void keyscan(void);
//unsigned char keyMaping(void);
int keyMaping(unsigned char *__key, unsigned char *press/** touched or released **/);
void fingerCheck(void);
void PeripheralInput_Check(void);
u8 wakeupCheck(void);
u16 PeripheralEvent_Check(void);

void VOPPort_DeInit(void);
void VOPPort_Init(void);
void VOP_Init(void);
void vop_busy(void);

void arrSort(unsigned int arr[],unsigned char n);
#if	1
unsigned int arrAverage(unsigned int arr[], unsigned char start, unsigned char end);
#endif

void VoltageDetection_Init(void);

void beep(void);
void bbeep(void);
void bbbeep(void);
void lbeep(void);
//void vp_play(u8 __vpIdx);
void vp_stor(unsigned char __vpIdx);


void actionDelay(int _delay);
void promptDelay(int _delay);

#if	0
void Unlock(void);
void Lock(void);
#endif

void lockAction(void);
void ledWarn(int _delay);
//void ledPrompt(int _delay);
void ledPrompt(int ledNO, int _delay/** ÖÜÆÚ **/);
void led_on(int ledNO);
void led_off(int ledNO);
void led_Toggle(int ledNO);

void wdg_init(void);
void wdg_feed(void);

void MOTOPort_Init(void);
void moto_normal(void);
void moto_reverse(void);
void moto_poweroff(void);
/*************************************************************/

void LEDtest_on(void);
void LEDtest_off(void);
void LEDtest_Toggle(void);

/******************************************************************************/
void LEDPort_Init();
void LEDPort_DeInit();
/******************************************************************************/
void LED1_off(void);
void LED1_on(void);
void LED2_off(void);
void LED2_on(void);

void LED3_off(void);
void LED3_on(void);
void LED4_off(void);
void LED4_on(void);

void LED5_off(void);
void LED5_on(void);
void LED6_off(void);
void LED6_on(void);

void LED7_off(void);
void LED7_on(void);
void LED8_off(void);
void LED8_on(void);

void LED9_off(void);
void LED9_on(void);
void LED10_off(void);
void LED10_on(void);

void LED11_off(void);
void LED11_on(void);
void LED12_off(void);
void LED12_on(void);

void LEDall_off(void);
void LEDall_on(void);

void LED1_Toggle(void);
void LED2_Toggle(void);
void LED3_Toggle(void);
void LED4_Toggle(void);
void LED5_Toggle(void);
void LED6_Toggle(void);
void LED7_Toggle(void);
void LED8_Toggle(void);
void LED9_Toggle(void);
void LED10_Toggle(void);
void LED11_Toggle(void);
void LED12_Toggle(void);

/*******************************************************************************/
void BLUETOOTHPort_Init(void);
void bluetooth_PWRoff(void);
void bluetooth_PWRon(void);

void bluetooth_ATenable(void);
void bluetooth_ATdisable(void);
/*******************************************************************************/
#endif
