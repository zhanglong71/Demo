#include "stm8s.h"
#include "CONST.h"
#include "ctype.h"  
#include "macro.h"
#include "global.h"	
#include "debug.h"

#include "stm8s_clk.h"
#include "stm8s_tim1.h"
#include "stm8s_adc2.h"
#include "stm8s_spi.h"

#include "arch.h"
//#include "led.h"
#include "uart.h"
#include "exti.h"
#include "tim1.h"
#include "w25x16.h"

#include "l_voice.h"
#include "l_i2c_touch.h"
#include "l_i2c_datetime.h"
#include "l_fingprint.h"
#include "l_oled.h"
#include "driver.h"		  
#include "charQue.h"	 
#include "f_finger.h" 
#include "f_bluetooth.h"

/*******************************************************************************
 * ��ʼ��ʱ��
 *******************************************************************************/
 void ClkInit(void)
 {
 	CLK_DeInit();			//��λʱ�ӼĴ���
  	CLK_HSICmd(ENABLE);		//ʹ���ڲ�����ʱ��
  	//CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV4);//�����ڲ�����ʱ��(ʱ�ӷ�Ƶ��:4��Ƶ),ϵͳʱ��4MHz
  	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);	//�����ڲ�����ʱ��(ʱ�ӷ�Ƶ��:1��Ƶ),ϵͳʱ��16MHz
  	CLK_ClockSecuritySystemEnable();				//����ʱ�Ӱ�ȫϵͳ
}

/*******************************************************************************
 * δʹ�õĶ˿�
 *******************************************************************************/
void unusePort_Init(void)
{
	 GPIO_Init(GPIOA, GPIO_PIN_1, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PA.1
	 GPIO_Init(GPIOA, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PA.2
	 #if	0
	 GPIO_Init(GPIOC, GPIO_PIN_1, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PC.1
	 GPIO_Init(GPIOC, GPIO_PIN_2, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PC.2
	 GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PC.3
	 GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_SLOW );	//PC.4
	 #endif
}
/** �ϵ��ʼ������ **/
void Peripheral_Init(void)
{
	ClkInit();
	
    //Test_WWDGReset();
    //WWDG_Configuration(); 
    unusePort_Init();
	Tim1_Init();
	USART_Configuration();
	LEDPort_Init();
	SPI_FLASH_Init();
    MOTOPort_Init();
    MSYSPOWER_INIT("�˿ڳ�ʼ��");
	FPort_Init();
	datetime_iic_port_init();
	datetimeDevice_init();
	//datetime_init();
    I2C_TOUCH_Init();
    EXIT_Init();
    OLED_SPI_Init();
    VOPPort_Init();
    VOP_Init();
	/** �����͵���� **/
	VoltageDetection_Init();
    MSYSPOWER_ON();		//�����͵�����ź�Դ
    MFPOWER_OFF();		//�ر�ָ�ƴ�����
	BLUETOOTHPort_Init();
}

/******************************************************************************
 * halt֮ǰ��׼���������ر��豸
 ******************************************************************************/
void Halt_OffDevice(void)
{
	led_off(0);			//�ر�led����
    OLED_POWEROFF();	//�ر�oled���Ե�Դ
    datetime_Deinit();
    MFPOWER_OFF();		//�ر�ָ�ƴ�������Դ
    MSYSPOWER_OFF();	//�رյ͵����Դ
    bluetooth_PWRoff();	//�ر�bluetooth
	//�ر��豸ǰ������ϵͳ��ʱ�Ӻ��ж�
	
	//ClkInit();
	SPI_Flash_PowerDown();
	ADC2_DeInit();		//STM8s207û��ADC1
	TIM1_DeInit();		//
	UART1_DeInit();		//STM8s207û��UART2, ��UART3
	
	/** Touch�˿� **/
	I2C_TOUCH_DeInit();
	//ͣ��ǰ�رղ���Ҫ�Ĺ���ģ���ʱ��
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI,DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2,DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART3,DISABLE);	//û�õ���ҲҪ�ر�
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3, DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER5, DISABLE);	//û�õ���ҲҪ�ر�
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER6, DISABLE);	//û�õ���ҲҪ�ر�
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
	//CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU,DISABLE);		//û�õ���ҲҪ�ر�	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_CAN,DISABLE);		//û�õ���ҲҪ�ر�(stm8s207û���)
	
	/** OLED��ʾ��5��GPIO�˿�ȫ������ڣ�����Ϊ��ЧƬѡ״̬ **/
	
	OLED_SPI_DeInit();
	/** voice������ **/
	VOPPort_DeInit();
	/** LED���� **/
	LEDPort_DeInit();
	
	 //����LSI
	 #if	0
        CLK_LSICmd(ENABLE);
        //??????MVR,????????
        CLK_SlowActiveHaltWakeUpCmd(ENABLE);
    #endif
        //??FLASH??????(???active-halt???)
        FLASH_SetLowPowerMode(FLASH_LPMODE_POWERDOWN);
	IRQ_enable();
}

/*******************************************************************************
 * ���Ѻ�������
 *******************************************************************************/
void Halt_OnDevice(void)
{
	ClkInit();
	
    FLASH_SetLowPowerMode(FLASH_LPMODE_STANDBY);	//!!!!!!!!!!!!
	Tim1_Init();
	USART_Configuration();
	LEDPort_Init();
    SPI_Flash_WAKEUP();
	datetime_init();
	
    MOTOPort_Init();
	FPort_Init();
    I2C_TOUCH_Init();
    VoltageDetection_Init();
    MSYSPOWER_ON();	//�����͵������Դ
    //EXIT_Init();
    
    VOPPort_Init();
    VOP_Init();
    
    OLED_SPI_Init();
    OLEDReset_IC();
    OLEDInit_IC();
	BLUETOOTHPort_Init();
}

/*******************************************************************************
 * ��ʱ���Ѻ����������Ӷ�����������
 * �����軽�Ѳ�ͬ���ǣ�
 * 1.�رհ�������(ͨ��I/O)
 * 2.�ر�OLED��ʾ
 * 3.�ر�ָ�ƴ�������Դ
 * 4.
 *******************************************************************************/
void Halt_OnAWU(void)
{
	ClkInit();
	
    FLASH_SetLowPowerMode(FLASH_LPMODE_STANDBY);	//!!!!!!!!!!!!
	Tim1_Init();
	USART_Configuration();
	
    SPI_Flash_WAKEUP();
	datetime_init();
	
    MOTOPort_Init();
    
	FPort_Init();
	
    I2C_TOUCH_Init();
    VoltageDetection_Init();
    MSYSPOWER_ON();	//�����͵������Դ
    //EXIT_Init();
    
    VOPPort_Init();
    VOP_Init();
    
    OLED_SPI_Init();
    OLEDReset_IC();
    OLEDInit_IC();
    
	All_Screen(0);		//�ر�oled����
	//LEDPort_Init();
	led_off(0);		//�ر�led����
	MFPOWER_OFF();	//�ر�ָ�ƴ�������Դ
	//BLUETOOTHPort_Init();
}

/*******************************************************************************
 * 
 * �����¼����
 * �������
 * 1.�˿ڷ�����ƽ�仯�������ⲿ�¼�����
 * 2.�޶˿ڵ�ƽ�仯����Ϊ���Զ�����
 * 
 * Check PC.3 after the MCU core wakeup before Peripheral enable
 * delay f = 2MHz
 *			1us - 2cycle
 * 			1ms - 2,000cycle
 *			
 *	return:	0 - awu
 *			1 - FP
 *			2 - ...
 *			3 - 
 *******************************************************************************/
u8 wakeupCheck(void)
{
    u16 i, j;
	//msg_t msg;
    func_t func;
    u8 u8Cnt;
    u8 u8Ret;
	
#define	CLOOP_10US	(11)
#define	CLOOP_100US	(CLOOP_10US * 10)
#define	CLOOP_1MS	(CLOOP_10US * 100)
	
	while(1) {
		halt();
        bluetooth_PWRon();
        
        if((g_IT_flag & (1 << 5)) == (1 << 5)) {	/** Finger Print **/
			g_IT_flag &= ~(1 << 5);
			
			for(i = 0; i < 10; i++) {
				if(FP_WAKECHECK()) {
					j++;
				} else {
					break;
				}
			}
			
			if(j == 10) {
          		u8Ret = 1;
          	
          		g_tick = 0;
    			SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_INIT);
    	  
    			fstack_init(&g_fstack);
				func.func = f_finger;
    			fstack_push(&g_fstack, &func);
				goto CGOTO_EVENT;
			}
		} 
        
		for(i = 0; i < (CLOOP_1MS * 12); i++) {/** wait 12ms(BLE_LINK state is stable after 4ms sometimes). do nothing **/}	
		
		if((g_IT_flag & (1 << 4)) == (1 << 4)) {	/** AWU ? **/
			/** auto wake up **/
			g_IT_flag &= ~(1 << 4);
            u8Cnt = 0;
			/** 100ms check BLE event **/
            for(i = 0; i < 60/** 210ms **/; i++) {
            //for(i = 0; i < 32/**125ms**/; i++) {
				for(j = 0; j < (CLOOP_1MS); j++) {
					if(BLE_LINKED("check BLE status")) {
                        if(u8Cnt++ > 3) {
                        
          					g_tick = 0;
    						SetTimer_irq(&g_timer[0], TIMER_100MS, CMSG_BLELINK);
					  		
					  		fstack_init(&g_fstack);
    						func.func = f_bluetooth;
    						fstack_push(&g_fstack, &func);
                            
                            u8Ret = 0;
                      		goto CGOTO_EVENT;
                    	}
                    	
				  	} else {
                      	u8Cnt = 0;
                    }
				}
            }
            
		} else { //nop();       /** Peripheral event **/
            u8Ret = 0xff;
			goto CGOTO_EVENT;
		}
        bluetooth_PWRoff();
	}
CGOTO_EVENT:
	//nop();  /** nothing but deWarning! **/
    return  u8Ret;
}
/*******************************************************************************
 * ��ѹADC���
 *******************************************************************************/
void VoltageDetection_Init(void)
{
	ADC2_DeInit();

	/**< Continuous conversion mode */
	/**< Analog channel 7 */
	/**< Prescaler selection fADC2 = fcpu/18 */
	/**< Conversion from Internal TIM TRGO event */
	/** DISABLE ADC2_ExtTriggerState**/
	/**< Data alignment right */
	/**< Schmitt trigger disable on AIN7 */
	/**DISABLE ADC2_SchmittTriggerState*/
	#if	1
	ADC2_Init(ADC2_CONVERSIONMODE_CONTINUOUS , ADC2_CHANNEL_7, ADC2_PRESSEL_FCPU_D18,\
	ADC2_EXTTRIG_TIM, DISABLE, ADC2_ALIGN_RIGHT, ADC2_SCHMITTTRIG_CHANNEL7,DISABLE);
	#endif
	ADC2_ITConfig(DISABLE);

	ADC2_Cmd(ENABLE);
	ADC2_StartConversion();

	//Conversion_Value = ADC2_GetConversionValue();
}

/*******************************************************************************
 * �������
 *******************************************************************************/
void MOTOPort_Init(void)
{
  /*******
   * ���
   *******/
   GPIO_Init(MOTO1_PORT,MOTO1_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );//���忪������Ĺܽŵ�ģʽ
   GPIO_Init(MOTO2_PORT,MOTO2_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   
   moto_poweroff();
}

void MOTOPort_DeInit(void)
{
   GPIO_Init(MOTO1_PORT,MOTO1_PIN, GPIO_MODE_IN_FL_NO_IT );//���忪������Ĺܽŵ�ģʽ
   GPIO_Init(MOTO2_PORT,MOTO2_PIN, GPIO_MODE_IN_FL_NO_IT );
}

void moto_normal(void) {GPIO_WriteLow(MOTO2_PORT, MOTO2_PIN);GPIO_WriteHigh(MOTO1_PORT, MOTO1_PIN);}
void moto_reverse(void) {GPIO_WriteHigh(MOTO2_PORT, MOTO2_PIN);GPIO_WriteLow(MOTO1_PORT, MOTO1_PIN);}
void moto_poweroff(void) {GPIO_WriteLow(MOTO2_PORT, MOTO2_PIN);GPIO_WriteLow(MOTO1_PORT, MOTO1_PIN);}

/*******************************************************************************
 * LED����
 *******************************************************************************/
void LEDPort_Init(void)
{
	/**
   	 * LED����
   	 **/ 
   #if	1
   GPIO_Init(LED1_PORT,LED1_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );//����LED�Ĺܽŵ�ģʽ
   GPIO_Init(LED2_PORT,LED2_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED3_PORT,LED3_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED4_PORT,LED4_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED5_PORT,LED5_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED6_PORT,LED6_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED7_PORT,LED7_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED8_PORT,LED8_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED9_PORT,LED9_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED10_PORT,LED10_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED11_PORT,LED11_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   GPIO_Init(LED12_PORT,LED12_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );
   
   #else
   GPIO_Init(LED1_PORT,LED1_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );//����LED�Ĺܽŵ�ģʽ
   GPIO_Init(LED2_PORT,LED2_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED3_PORT,LED3_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED4_PORT,LED4_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED5_PORT,LED5_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED6_PORT,LED6_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED7_PORT,LED7_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED8_PORT,LED8_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED9_PORT,LED9_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED10_PORT,LED10_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED11_PORT,LED11_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   GPIO_Init(LED12_PORT,LED12_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   #endif
   //GPIO_Init(LEDtest_PORT,LEDtest_PIN,GPIO_MODE_OUT_PP_HIGH_FAST ); //?????????????????????????
}

void LEDPort_DeInit(void)
{
	/**
   	 * LED����
   	 *
   	 * do nothing(don`t set as input port )
   	 **/ 
   	
}

void LED1_off(void) {GPIO_WriteLow(LED1_PORT, LED1_PIN);}
void LED1_on(void) {GPIO_WriteHigh(LED1_PORT, LED1_PIN);}

void LED2_off(void) {GPIO_WriteLow(LED2_PORT, LED2_PIN);}
void LED2_on(void) {GPIO_WriteHigh(LED2_PORT, LED2_PIN);}

void LED3_off(void) {GPIO_WriteLow(LED3_PORT, LED3_PIN);}
void LED3_on(void) {GPIO_WriteHigh(LED3_PORT, LED3_PIN);}

void LED4_off(void) {GPIO_WriteLow(LED4_PORT, LED4_PIN);}
void LED4_on(void) {GPIO_WriteHigh(LED4_PORT, LED4_PIN);}

void LED5_off(void) {GPIO_WriteLow(LED5_PORT, LED5_PIN);}
void LED5_on(void) {GPIO_WriteHigh(LED5_PORT, LED5_PIN);}

void LED6_off(void) {GPIO_WriteLow(LED6_PORT, LED6_PIN);}
void LED6_on(void) {GPIO_WriteHigh(LED6_PORT, LED6_PIN);}

void LED7_off(void) {GPIO_WriteLow(LED7_PORT, LED7_PIN);}
void LED7_on(void) {GPIO_WriteHigh(LED7_PORT, LED7_PIN);}

void LED8_off(void) {GPIO_WriteLow(LED8_PORT, LED8_PIN);}
void LED8_on(void) {GPIO_WriteHigh(LED8_PORT, LED8_PIN);}

void LED9_off(void) {GPIO_WriteLow(LED9_PORT, LED9_PIN);}
void LED9_on(void) {GPIO_WriteHigh(LED9_PORT, LED9_PIN);}

void LED10_off(void) {GPIO_WriteLow(LED10_PORT, LED10_PIN);}
void LED10_on(void) {GPIO_WriteHigh(LED10_PORT, LED10_PIN);}

void LED11_off(void) {GPIO_WriteLow(LED11_PORT, LED11_PIN);}
void LED11_on(void) {GPIO_WriteHigh(LED11_PORT, LED11_PIN);}

void LED12_off(void) {GPIO_WriteLow(LED12_PORT, LED12_PIN);}
void LED12_on(void) {GPIO_WriteHigh(LED12_PORT, LED12_PIN);}

void LED1_Toggle(void){GPIO_WriteReverse(LED1_PORT,LED1_PIN);}
void LED2_Toggle(void){GPIO_WriteReverse(LED2_PORT,LED2_PIN);}
void LED3_Toggle(void){GPIO_WriteReverse(LED3_PORT,LED3_PIN);}
void LED4_Toggle(void){GPIO_WriteReverse(LED4_PORT,LED4_PIN);}
void LED5_Toggle(void){GPIO_WriteReverse(LED5_PORT,LED5_PIN);}
void LED6_Toggle(void){GPIO_WriteReverse(LED6_PORT,LED6_PIN);}
void LED7_Toggle(void){GPIO_WriteReverse(LED7_PORT,LED7_PIN);}
void LED8_Toggle(void){GPIO_WriteReverse(LED8_PORT,LED8_PIN);}
void LED9_Toggle(void){GPIO_WriteReverse(LED9_PORT,LED9_PIN);}
void LED10_Toggle(void){GPIO_WriteReverse(LED10_PORT,LED10_PIN);}
void LED11_Toggle(void){GPIO_WriteReverse(LED11_PORT,LED11_PIN);}
void LED12_Toggle(void){GPIO_WriteReverse(LED12_PORT,LED12_PIN);}

void LEDall_off(void)
{
	LED1_off();
	LED2_off();
	LED3_off();
	LED4_off();
	
	LED5_off();
	LED6_off();
	LED7_off();
	LED8_off();
	
	LED9_off();
	LED10_off();
	LED11_off();
	LED12_off();
}
void LEDall_on(void)
{
	LED1_on();
	LED2_on();
	LED3_on();
	LED4_on();
	
	LED5_on();
	LED6_on();
	LED7_on();
	LED8_on();
	
	LED9_on();
	LED10_on();
	LED11_on();
	LED12_on();
}

void LEDall_Toggle(void)
{
	LED1_Toggle();
	LED2_Toggle();
	LED3_Toggle();
	LED4_Toggle();
	
	LED5_Toggle();
	LED6_Toggle();
	LED7_Toggle();
	LED8_Toggle();
	
	LED9_Toggle();
	LED10_Toggle();
	LED11_Toggle();
	LED12_Toggle();
}

paction_t_0 ledarr_on[] = {
	LEDall_on,
	LED1_on,
	LED2_on,
	LED3_on,
	LED4_on,
	LED5_on,
	LED6_on,
	LED7_on,
	LED8_on,
	LED9_on,
	LED10_on,
	LED11_on,
	LED12_on,
};

paction_t_0 ledarr_off[] = {
	LEDall_off,
	LED1_off,
	LED2_off,
	LED3_off,
	LED4_off,
	LED5_off,
	LED6_off,
	LED7_off,
	LED8_off,
	LED9_off,
	LED10_off,
	LED11_off,
	LED12_off,
};

paction_t_0 ledarr_Toggle[] = {
	LEDall_Toggle,
	LED1_Toggle,
	LED2_Toggle,
	LED3_Toggle,
	LED4_Toggle,
	LED5_Toggle,
	LED6_Toggle,
	LED7_Toggle,
	LED8_Toggle,
	LED9_Toggle,
	LED10_Toggle,
	LED11_Toggle,
	LED12_Toggle,
};

void led_on(int ledNO)
{
	ledarr_on[ledNO]();
}

void led_off(int ledNO)
{
	ledarr_off[ledNO]();
}

void led_Toggle(int ledNO)
{
	ledarr_Toggle[ledNO]();
}

/*******************************************************************************
 * BLUETOOTH����
 *******************************************************************************/
void BLUETOOTHPort_Init(void)
{
	/**
   	 * BLUETOOTH
   	 **/ 
   GPIO_Init(BLE_POWER_CTR_PORT, BLE_POWER_CTR_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);	//BLUETOOTH POWER CTR(O)
   GPIO_Init(BLE_AT_CTR_PORT, BLE_AT_CTR_PIN, GPIO_MODE_OUT_PP_HIGH_SLOW);			//BLUETOOTH AT CTR(O)
   //GPIO_Init(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN, GPIO_MODE_IN_PU_IT);
   GPIO_Init(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN, GPIO_MODE_IN_PU_NO_IT);	      	//Link state check(I)
   
   bluetooth_ATdisable();
   //bluetooth_PWRoff();
   bluetooth_PWRon();
}

void BLUETOOTHPort_DeInit(void)
{
}

void bluetooth_PWRoff(void) {GPIO_WriteLow(BLE_POWER_CTR_PORT, BLE_POWER_CTR_PIN);}
void bluetooth_PWRon(void) {GPIO_WriteHigh(BLE_POWER_CTR_PORT, BLE_POWER_CTR_PIN);}

void bluetooth_ATenable(void) {GPIO_WriteLow(BLE_AT_CTR_PORT, BLE_AT_CTR_PIN);}
void bluetooth_ATdisable(void) {GPIO_WriteHigh(BLE_AT_CTR_PORT, BLE_AT_CTR_PIN);}

//#define MBLUETOOTH_LINKCHK()  (GPIO_ReadInputPin(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN) != 0) //����Ч
#define MBLUETOOTH_LINKCHK()  (GPIO_ReadInputPin(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN) == 0) //����Ч
/*******************************************************************************
 * specify the state and 
 *******************************************************************************/
void portAction(paction_t_0 __func, u16 _delay, msgType_t __msgType)
{
	action_t action;
    action.actionType = CACT_MISC;
    action.actionTime = _delay;
    action.func = __func;
    actionQueueIn(&g_blinkQueue, &action);
}
/** ָʾ�ƶ�����ʾ **/
void ledPrompt(int ledNO, int _delay/** ���� **/)
{
	action_t action;
    action.actionType = CACT_LEDPMTON;
    action.actionTime = _delay;
    action.func = ledarr_on[ledNO];
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDPMTOFF;
    action.actionTime = _delay;
    action.func = ledarr_off[ledNO];
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDPMTON;
    action.actionTime = _delay;
    action.func = ledarr_on[ledNO];
    actionQueueIn(&g_blinkQueue, &action);
}

/** ָʾ�ƾ�����ʾ **/
void ledWarn(int _delay)	/** ������ʾ **/
{
	action_t action;
    action.actionType = CACT_LEDWRNON;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDWRNOFF;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDWRNON;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDWRNOFF;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDWRNON;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
    
    action.actionType = CACT_LEDWRNOFF;
    action.actionTime = _delay;
    actionQueueIn(&g_blinkQueue, &action);
}
/*******************************************************************************
 * value_inc()/value_dec()
 * ��ָ���ı�����[_max, _min]�����ڼ�/��1. 
 *******************************************************************************/
int value_inc(int _value, int _max, int _min)
{
	if(_value >= _max) {
		_value = _min;
	} else {
		_value += 1;
	}
	
	return	_value;
}

int value_dec(int _value, int _max, int _min)
{
	if(_value <= _min) {
		_value = _max;
	} else {
		_value -= 1;
	}
	
	return	_value;
}

/*******************************************************************************
 * value_BCDinc()/value_BCDdec()
 * ��ָ���ı�����[99, 0]�����ڼ�/��1. 
 *******************************************************************************/
u8 BCDvalue_inc(u8 _value, u8 _max, u8 _min)
{
	if(_value >= _max) {
		_value = _min;
	} else {
		_value += 1;
	}
	
	return	_value;
}

u8 BCDvalue_dec(u8 _value, u8 _max, u8 _min)
{
	if(_value <= _min) {
		_value = _max;
	} else {
		_value -= 1;
	}
	
	return	_value;
}

/*******************************************************************************
 * DAEMON_USART1_Send()/DAEMON_USART1_Recive()
 *******************************************************************************/
void DAEMON_USART1_Send(charBuf_queue_t* comTxQue)
{
    charData_t   comData;
    //static unsigned char iTmp;

    if(UART1_GetFlagStatus(UART1_FLAG_TXE) != RESET)  {
        if(charQueueOut_irq(comTxQue, &comData) == TRUE)   {
            UART1_SendData8(comData.ucVal);
        }
    }
}

#if	0
void DAEMON_USART1_Recive(charBuf_queue_t* comRxQue)
{
    charData_t   comData;
    //static unsigned char iTmp;

    if(UART1_GetFlagStatus(UART1_FLAG_RXNE) != RESET) {
    	g_T_UART1Busy = 0;
        comData.ucVal = UART1_ReceiveData8();
        charQueueIn_irq(comRxQue, &comData);
    }
}
#endif

/*******************************************************************************
 * DAEMON_USART3_Send()/DAEMON_USART3_Recive()
 *******************************************************************************/
void DAEMON_USART3_Send(charBuf_queue_t* comTxQue)
{
    charData_t   comData;
    //static unsigned char iTmp;

    if(UART3_GetFlagStatus(UART3_FLAG_TXE) != RESET)  {
        if(charQueueOut_irq(comTxQue, &comData) == TRUE)   {
            UART3_SendData8(comData.ucVal);
        }
    }
}

void DAEMON_USART3_Recive(charBuf_queue_t* comRxQue)
{
    charData_t   comData;
    //static unsigned char iTmp;

    if(UART3_GetFlagStatus(UART3_FLAG_RXNE) != RESET) {
    	//Timer_comBusy = 0;
    	
    	g_T_UART3Busy = 0;
        comData.ucVal = UART3_ReceiveData8();
        charQueueIn_irq(comRxQue, &comData);
    }
}

/******************************************
 * description: g_iic2 ==> (__key, __press)
 * input: g_iic2
 * output: (__key, __press)
 * return: success/fail
 ******************************************/
int keyMaping(unsigned char *__key, unsigned char *__press/** touched or released **/)
{
	int	ret = 0;
	unsigned short	raw_key = g_iic2.addr | ((g_iic2.data & 0x0f) << 8);
	
	if((__key == 0) || (__press == 0)) {
		return	0;
	}	
	*__press = (g_iic2.data & 0x10);
	
	if(raw_key != 0) {
		ret = 1;
		switch(raw_key)
		{
		case	0x800:	*__key = CKEY_9;	break;		//T12	
		case	0x400:	*__key = CKEY_pound;break;		//T11
		case	0x200:	*__key = CKEY_3;	break;		//T10	
		case	0x100:	*__key = CKEY_6;	break;		//T9
		case	0x080:	*__key = CKEY_2;	break;		//T8
		case	0x040:	*__key = CKEY_1;	break;		//T7
		case	0x020:	*__key = CKEY_5;	break;		//T6
		case	0x010:	*__key = CKEY_4;	break;		//T5
		case	0x008:	*__key = CKEY_7;	break;		//T4
		case	0x004:	*__key = CKEY_asterisk;break;	//T3
		case	0x002:	*__key = CKEY_0;	break;		//T2
		case	0x001:	*__key = CKEY_8;	break;		//T1
		default:	ret = 0;				break;
		}
	} else {
		if(*__press != 0) {			//��ָ����
			ret = 1;
			*__key = CKEY_WU;
		}
	}
	return	ret;
}

void keyscan(void)
{
	static unsigned char ucKey = 0;
	u8 u8tmp;
	u8 ucTouRele;
	msg_t msg;
//	int i;
	
	/** ��ⰴ��״̬�ı仯 **/
	if(KB_WAKECHECK(x)) {  //��⵽����
		
		if((g_flag & (1 << 7)) == (1 << 7))	{	//�Ѿ�����
            if(I2C_TOUCH_Read((u8 *)&g_iic2, 0, 4)) {
				keyMaping(&u8tmp, &ucTouRele);
				ucKey = u8tmp;
				
				if(g_tmr_key == 0){
					#if	1
					if((ucTouRele != 0) && (ucKey != 0)) {
						g_tmr_key = TIMER_100MS;
						
						msg.msgValue = ucKey;
						msg.msgType = CMSG_DKEY;
						//inq_irq(&g_msgq, &msg);
						msgq_in_irq(&g_msgq, &msg);
					}
					#endif
				}
            } else {
            	/** iic read failed, reset it **/
            	#if	1
            	 I2C_TOUCH_Init();
            	#else
				//msg.msgValue = ucKey;
				msg.msgType = CIIC_INIT;
				inq_irq(&g_msgq, &msg);
				#endif
            }
		} else {
			g_flag |= (1 << 7);
			
			//msg.msgType = CMSG_BACK;	//!!!!!!!!!!!!!!!!!
			//inq_irq(&g_msgq, &msg);		//!!!!!!!!!!!!!!!!!
			
			g_tmr_key = TIMER_500MS;
			I2C_TOUCH_Read((u8 *)&g_iic2, 0, 2);	
		}
	}
	
	/***************************************************************************
	 * g_flag.12 the last state of setkey(before jitter)
	 * g_flag.13 the last state of setkey(after jitter)
	 * 
	 * note: the SET_WAKECHECK() is volatile
	 * 
	 ***************************************************************************/
	 #if	0
	u8tmp = ((g_flag >> 12) & 0x03);
	if(u8tmp == 0) {
		/** idle(released) **/
		if(SET_WAKECHECK("�������ü�")) {
			g_tmr_setkey = TIMER_50MS;
			g_flag |= (1 << 12);
		}
	} else if(u8tmp == 1) {
		/** pressed counter **/
		if(SET_WAKECHECK("�������ü�")) {
			if(g_tmr_setkey == TIMER_KEYCHK) {//setKey press timeout
				g_flag |= (1 << 13);
				
				msg.msgValue = CKEY_SET;
				msg.msgType = CMSG_DKEY;
				msgq_in_irq(&g_msgq, &msg);
			}
		} else {
			g_flag &= ~(1 << 12);
		}
	} else if(u8tmp == 2) {
		/** released counter **/
		if(SET_WAKECHECK("�������ü�")) {
			g_flag |= (1 << 12);
		} else {
			if(g_tmr_setkey == TIMER_KEYCHK) { //setKey release timeout
				g_flag &= ~(1 << 13);
				
				msg.msgValue = CKEY_SET;
				msg.msgType = CMSG_UKEY;
				msgq_in_irq(&g_msgq, &msg);
			}
		}
	} else if(u8tmp == 3) {
		/** pressed **/
		if(SET_WAKECHECK("�������ü�")) {
			/** do nothing or key timer **/
		} else {
			g_tmr_setkey = TIMER_50MS;
			g_flag &= ~(1 << 12);
		}
	}
	
	#endif
	
}

/*******************************************************************************
 * fingerCheck()
 *******************************************************************************/
void fingerCheck(void)
{
	#if	0
	msg_t msg;
	/** ����ָ��ͷ **/
	if(FP_WAKECHECK()) {
		if((g_flag & (1 << 6)) == 0) {
			msg.msgType = CFIGER_ON;
			msgq_in_irq(&g_msgq, &msg);
		}
		g_flag |= (1 << 6);
	} else {
		g_flag &= ~(1 << 6);
	}
	#endif
}

/*******************************************************************************
 * PeripheralInput_Check
 *
 * ����˿ڼ��
 * GPIOB(1,2,3,4,5)
 * GPIOE(4)
 * ƴ�ճ�״̬��:        0b000x 0000 00xx xxx0
 * ״̬���룺	 0x103E(0b0001 0000 0011 1110)
 * ���¼�״̬��: 0x003A(0b0000 0000 0011 1010)   
 * 
 * �����¼���:   0x0038(0b0000 0000 0011 1000) - 0b0000 0000 0000 00x0
 * ң���¼���:   0x003E(0b0000 0000 0011 1110) - 0b0000 0000 0000 0x00
 * �����¼���:   0x0032(0b0000 0000 0011 0010) - 0b0000 0000 0000 x000
 * ����������:   0x002A(0b0000 0000 0010 1010) - 0b0000 0000 000x 0000
 * ���ð�����:   0x001A(0b0000 0000 0001 1010) - 0b0000 0000 00x0 0000
 * ָ�ƶ�����:   0x103A(0b0001 0000 0011 1010) - 0b000x 0000 0000 0000
 * 
 * Ŀǰ�õ���433/315ģ����յ��źź󣬻ᷢ��2��500ms�ĸ����壬���250ms. ��󷵻ص��͵�ƽ״̬
 *******************************************************************************/
void PeripheralInput_Check(void)
{
#define CINPUTMASK	(0x103E)
#define CINPUTIDLE	(0x003A)	//(0x003C)
#define	MisINPUTBUSY	(portVal != CINPUTIDLE)

#define MisPFSENSOR		(lastPortVal == 0x103A)
#define MisSETKEY		(lastPortVal == 0x001A)
#define MisVOPKEY		(lastPortVal == 0x002A)
#define MisTOUCHKEY		(lastPortVal == 0x0032)
#define MisREMOTEKEY	(lastPortVal == 0x003E)
#define MisPRYKEY		(lastPortVal == 0x0038)
	msg_t msg;
	u8 u8tmp;
	u16 portVal;
	static u16 lastPortVal;
	
	portVal = (((GPIO_ReadInputData(GPIOE) << 8) | GPIO_ReadInputData(GPIOB)) & CINPUTMASK);
	
	u8tmp = ((g_flag >> 12) & 0x03);
	if(u8tmp == 0) {
		/** idle(released) **/
		//if(SET_WAKECHECK("�������ü�")) {
		if(MisINPUTBUSY) {
			g_tmr_setkey = TIMER_50MS;
			g_flag |= (1 << 12);
		}
	} else if(u8tmp == 1) {
		/** pressed counter **/
		//if(SET_WAKECHECK("�������ü�")) {
		if(MisINPUTBUSY) {
			if(g_tmr_setkey == TIMER_KEYCHK) {//setKey press timeout
				g_flag |= (1 << 13);
				lastPortVal = portVal;
				
				if(MisPFSENSOR) {
					msg.msgType = CFIGER_ON;
				} else if(MisSETKEY) {
					msg.msgValue = CKEY_SET;
					msg.msgType = CMSG_DKEY;
				} else if(MisVOPKEY) {
					msg.msgValue = CKEY_VOP;
					msg.msgType = CMSG_DKEY;
				} else if(MisTOUCHKEY) {
					msg.msgType = CMSG_NONE;
				} else if(MisREMOTEKEY) {
					msg.msgType = CMSG_RMT;
				} else if(MisPRYKEY) {
					msg.msgType = CMSG_PRY;
				} else {
					msg.msgType = CMSG_NONE;
				}
				
				msgq_in_irq(&g_msgq, &msg);
			}
		} else {
			g_flag &= ~(1 << 12);
		}
	} else if(u8tmp == 2) {
		/** released counter **/
		//if(SET_WAKECHECK("�������ü�")) {
		if(MisINPUTBUSY) {
			g_flag |= (1 << 12);
		} else {
			if(g_tmr_setkey == TIMER_KEYCHK) { //setKey release timeout
				g_flag &= ~(1 << 13);
				
				if(MisSETKEY) {
					msg.msgValue = CKEY_SET;
					msg.msgType = CMSG_UKEY;
				} else if(MisVOPKEY) {
					msg.msgValue = CKEY_VOP;
					msg.msgType = CMSG_UKEY;
				} else if(MisREMOTEKEY) {
					//msg.msgType = CMSG_RMT;
				} else {
					msg.msgType = CMSG_NONE;
				}
				msgq_in_irq(&g_msgq, &msg);
			}
		}
	} else if(u8tmp == 3) {
		/** pressed **/
		//if(SET_WAKECHECK("�������ü�")) {
		if(MisINPUTBUSY) {
			/** do nothing or key timer **/
		} else {
			g_tmr_setkey = TIMER_50MS;
			g_flag &= ~(1 << 12);
		}
	}
	
#undef CINPUTMASK
#undef CINPUTIDLE
#undef MisINPUTBUSY
#undef MisPFSENSOR	
#undef MisSETKEY
#undef MisVOPKEY
#undef MisTOUCHKEY
#undef MisREMOTEKEY
#undef MisPRYKEY
}

/*******************************************************************************
 * ���������Ƿ�����Ч�¼� 
 *
 * �������ƺ�û�õ�
 *******************************************************************************/
u16 PeripheralEvent_Check(void)
{
	u16 portFlag = 0;
	
	if(FP_WAKECHECK("finger Print")) 	{portFlag |= (1     );} else {portFlag &= (u16)~(1     );}
	if(KB_WAKECHECK("touch key   ")) 	{portFlag |= (1 << 1);} else {portFlag &= (u16)~(1 << 1);}
	if(SET_WAKECHECK("set key    "))	{portFlag |= (1 << 2);} else {portFlag &= (u16)~(1 << 2);}
	if(MUSIC_WAKECHECK("music key"))	{portFlag |= (1 << 3);} else {portFlag &= (u16)~(1 << 3);}
	if(REMOTE_WAKECHECK("remt key")) 	{portFlag |= (1 << 4);} else {portFlag &= (u16)~(1 << 4);}
	if(FANGQIAO_WAKECHECK("anti-theft")){portFlag |= (1 << 5);} else {portFlag &= (u16)~(1 << 5);}
	
	return	portFlag;
}

/*******************************************************************************
 * �����˿ڳ�ʼ��������Ϊ�������
 *******************************************************************************/
void VOPPort_Init(void)
{
	GPIO_Init(VOICE_DATA_PORT, VOICE_DATA_PIN, GPIO_MODE_OUT_PP_HIGH_FAST );//����LED�Ĺܽŵ�ģʽ
	GPIO_Init(VOICE_RST_PORT, VOICE_RST_PIN, GPIO_MODE_OUT_PP_HIGH_FAST );
}

/*********************************************************************
 * ����͹���״̬��(��Щ�˿���Ϊ��������˿�) 
 *********************************************************************/
void VOPPort_DeInit(void)
{
  #if	1
	MVOPRESET_H();
	MVOPDATA_H();
  #else
	//GPIO_Init(VOICE_DATA_PORT, VOICE_DATA_PIN, GPIO_MODE_IN_FL_NO_IT );
	//GPIO_Init(VOICE_RST_PORT, VOICE_RST_PIN, GPIO_MODE_IN_FL_NO_IT );
	//GPIO_Init(VOICE_BUSY_PORT, VOICE_BUSY_PIN, GPIO_MODE_IN_FL_NO_IT );
  #endif
}

/** ������λ **/
void VOP_Init(void)
{
  #if	1
	MVOPRESET_L();
   	//MVOPDATA_H();
	delaySpin_ms(10);
    MVOPRESET_H();
	delaySpin_ms(10);
	MVOPRESET_L();
	delaySpin_ms(10);
    //MVOPRESET_H();
  #endif
}

/*******************************************************************************
 * vop_busy()��������
 *	(CVOP_NY3P --- һ���ɸ�(�쵽2��)����(�쵽200��)�����䣬��ʾ���Ž���)
 *******************************************************************************/
void vop_busy(void)
{
#if	0
	//#define	CMAX_BUSY	100
	#define	CMAX_IDLE	20
	msg_t msg;
	static u8 u8Count = 0;
	
	//static u8 u8VopBusyFlag = 0;
	/** ������busy�ź� H --> L
	 * Ϊ��ֹ�����⼰�����쳣��������������������⴦��
	 * 1. һ������ָ�ʼ���g_tmr_iVopBusy����, ֱ��������TIMER_VOPBUSY��ſ�ʼ���
	 * 2. g_tmr_iVopBusy������TIMER_VOPBUSY���busy״̬
	 * 3. busy����ָ��ʱ������λ
	 * 4. idle����ָ��ʱ������λ
	 *
	 * ���ˣ����bit0, 3��bit1, 2��Ч���ʾ�е�ƽ�仯
	 * 2��3ͬʱ��Ч����ʾ���ȶ��ĵ�ƽ�仯
	 **/
	 
	if(g_tmr_iVopBusy == TIMER_VOPBUSY) {	/** �������������Ѿ���� **/
		if(MVOP_BUSYCHECK()) {	
			u8Count = 0;
		} else {
			if(++u8Count > CMAX_IDLE) {
				msg.msgType = CPMT_TOUT;
				//inq_irq(&g_msgq, &msg);
				msgq_in_irq(&g_msgq, &msg);
				
				u8Count = 0;
				g_tmr_iVopBusy = TIMER_VOPBUSY + 1;
			}
		}
	} /** else (g_tmr_iVopBusy > 0) **/
#endif
}

/*******************************************************************************
 * ��������: �����µĶ���
 *******************************************************************************/
void adcSample_Start(adcData_t *adcData)
{
	SetTimer_irq(adcData->timer, adcData->tick, adcData->stepMsgType);
	
	adcData->u16count = 0;
	adcData->u16currValue = 0;
	adcData->u16average = 0;
	
}
void adcSample_Stop(adcData_t *adcData)
{
	ClrTimer_irq(adcData->timer);
}

/*******************************************************************************
 * ��������: �����µĶ���
 *******************************************************************************/
static void startAction(actionQueue_t *q, action_t * pAction)
{
	SetTimer_irq(q->timer, pAction->actionTime, q->stepMsgType);
	
	switch(pAction->actionType)
	{
	/** ����ת��(���Դ��������Ϣ) **/	
    case    CACT_MOTONORM:	//��ת
    case	CACT_MOTOREVE:	//��ת
    case	CACT_MOTOWAIT:	//ͣת
		q->flag = (1<<7);
        if(pAction->func != 0)pAction->func();
		break;	
		
	/** ָʾ����ʾ��˸(��sys���������Ϣ) **/
	case	CACT_LEDPMTOFF:
	case	CACT_LEDPMTON:
	case	CACT_LEDPMTWAT:
		q->flag = (1<<6);
        if(pAction->func != 0)pAction->func();
		break;	
		
	/** ָʾ�ƾ�����˸(���Թ��뵽��ʾ��˸��) **/		
	case	CACT_LEDWRNON:
	case	CACT_LEDWRNOFF:
	case	CACT_LEDWRNWAT:
		q->flag = (1<<5);
        if(pAction->func != 0)pAction->func();
		break;
		
	/** beep������(���Թ��뵽��ʾ��˸��) **/
	case	CACT_BEEPON:
		q->flag = (1<<4);
		MVOPDATA_H();
		break;		
	case	CACT_BEEPOFF:
		q->flag = (1<<4);
		MVOPDATA_L();
		break;
		
	/** voiceprompt���� **/
	case	CACT_VOPWAT:
		q->flag = (1<<3);
		break;
	case	CACT_VOPON:
		q->flag = (1<<2);
		//vp_play((unsigned char)pAction->actionPara);
		//func = (void (*)(u8 __vpIdx))pAction->func;
        //func((unsigned char)pAction->actionPara);
        if(pAction->func != 0)((void (*)(u8 __vpIdx))pAction->func)((unsigned char)pAction->actionPara);
		break;
		
	/** misc�������� **/	
	case	CACT_MISC:
		q->flag = (1<<1);
		if(pAction->func != 0)pAction->func();
		break;
			
	default:
		/** ����ʶ��Ķ��� **/
		q->flag = 0;
		break;
	}
}

/*******************************************************************************
 * ��������ֹͣ��ǰ�Ķ���
 * Ŀǰ���������ʱ����������
 *
 * (�����˿������������ԭ��ʲôʱ�����𣬼ǵò��ϣ���Ҫ������ʼ��һ������)
 *
 * �ƺ�Ӧ���ں������һ���ƺ��������Իָ�������״̬��������������������������
 *******************************************************************************/
//static void stopAction(actionQueue_t *q)
void stopAction(actionQueue_t *q)
{
	ClrTimer_irq(q->timer);
		/** stop the timeout action  **/
	if(q->flag & (1<<7)) {
		//moto_poweroff();
		q->flag &= ~(1<<7);
	} else if(q->flag & (1<<6)) {
		//MOPBlue_off();
		q->flag &= ~(1<<6);
	} else if(q->flag & (1<<5)) {
		//MOPRed_off();
		q->flag &= ~(1<<5);
	} else if(q->flag & (1<<4)) {
		MVOPDATA_L();
		q->flag &= ~(1<<4);
	} else if(q->flag & (1<<3)) {
		/** delay... do Nothing **/
		q->flag &= ~(1<<3);
	} else if(q->flag & (1<<2)) {
		/** stop VP, send stop command when playing **/
		//if(GPIO_ReadInputPin(GPIOB, GPIO_PIN_1) == 1)/* vp_play((unsigned char)CVOP_STOP) */;
		vp_play(CVOPID_RESET);
		q->flag &= ~(1<<2);
	} else if(q->flag & (1<<1)) {
		/** misc **/
		q->flag &= ~(1<<1);
	} else {
		/** nothing **/
	}
}

#if	1
 /** ������ƶ������У�����ֹͣ��ǰ����. ֻ���ϵ�ʱ���� **/
void blinkInit(void)
{
	actionQueueInit(&g_blinkQueue, &(g_timer[3]), CBLK_TOUT, CBLK_OVER);
	stopAction(&g_blinkQueue);
}

 /** ��������������У�����ֹͣ��ǰ�������� **/
void promptInit(void)
{
	actionQueueInit(&g_promptQueue, &(g_timer[2]), CPMT_TOUT, CPMT_OVER);
	stopAction(&g_promptQueue);
}

 /** ��ն������У�����ֹͣ��ǰ���� **/
void actionInit(void)
{
	actionQueueInit(&g_actionQueue, &(g_timer[1]), CACT_TOUT, CACT_OVER);
	stopAction(&g_actionQueue);
}
#endif

/** �¼����д��� **/
void actionDoing(actionQueue_t * q)
{
	action_t action;
	
	if(q->flag == 0) {
		if(actionQueueOut(q, &action) == TRUE) {
			startAction(q, &action);
		}
	}
}

/** ϵͳ��ǰ�¼����� **/
void actProcess(actionQueue_t * q)
{
	action_t action;
	
	/** stop the timeout action first  **/
	stopAction(q);
	
	/** start the next action **/
	if(actionQueueOut(q, &action) == TRUE) {
		startAction(q, &action);
	} else {
		//if(q->flag != 0) {
		if(1) {
			q->flag = 0;
			SetTimer_irq(q->timer, TIMER_100MS, q->overMsgType);
		}
	}
}

/*******************************************************************************
 * ��8bit�ܱ�����8000��(13bit�ܱ������ֵ8192)��8bit�ı�����Ҫ����5λ
 * ��ÿ��λ���32ms
 * ԭ���ϣ�������Ϊ������ÿ��256ms(8)
 *******************************************************************************/
const u8 u8VopLen32Time_tab[] = {
/*0*/	0,	//��
/*1*/	1,	//silent
/*2*/	4,	//beep- 32*4 = 128ms
/*3*/	(8 * 1),	//�� - 32*8 = 256ms
/*4*/	(8 * 1),	//Ч - 32*8 = 256ms
/*5*/	(8 * 1),	//�� - 32*8 = 256ms
/*6*/	(8 * 1),	//*  - 32*8 = 256ms
/*7*/	(8 * 1),	//#  - 32*8 = 256ms
/*8*/	(8 * 1),	//�� - 32*8 = 256ms
/*9*/	(8 * 1),	//�� - 32*8 = 256ms

/*10*/	(8 * 1),	//�� - 32*8 = 256ms
/*11*/	(8 * 1),	//�� - 32*8 = 256ms
/*12*/	(8 * 1),	//�� - 32*8 = 256ms
/*13*/	(8 * 1),	//�� - 32*8 = 256ms
/*14*/	(8 * 1),	//�� - 32*8 = 256ms
/*15*/	(8 * 2),	//���� - 32*(8 * 2) = 512ms
/*16*/	(8 * 2),	//ͨ�� - 32*16 = 512ms
/*17*/	(8 * 2),	//���� - 32*16 = 512ms
/*18*/	(8 * 2),	//��ָ - 32*16 = 512ms
/*19*/	(8 * 2),	//��� - 32*16 = 512ms

/*20*/	(8 * 2),	//ɾ�� - 32*16 = 512ms
/*21*/	(8 * 2),	//���� - 32*16 = 512ms
/*22*/	(8 * 2),	//ϵͳ - 32*16 = 512ms
/*23*/	(8 * 2),	//���� - 32*16 = 512ms
/*24*/	(8 * 2),	//��֤ - 32*16 = 512ms
/*25*/	(8 * 2),	//�ɹ� - 32*16 = 512ms
/*26*/	(8 * 2),	//ʧ�� - 32*16 = 512ms
/*27*/	(8 * 2),	//ָ�� - 32*16 = 512ms
/*28*/	(8 * 2),	//��¼ - 32*16 = 512ms
/*29*/	(8 * 2),	//���� - 32*16 = 512ms !!!!!!!!!!!!!!!!!!!!!!!!!

/*30*/	(8 * 2),	//�Ժ� - 32*16 = 512ms
/*31*/	(8 * 2),	//��� - 32*16 = 512ms
/*32*/	(8 * 1),	//2 - 32*8 = 256ms
/*33*/	(8 * 2),	//���� - 32*16 = 512ms
/*34*/	(8 * 2),	//ȷ�� - 32*16 = 512ms
/*35*/	(8 * 2),	//ȡ�� - 32*16 = 512ms
/*36*/	(8 * 2),	//�û� - 32*16 = 512ms
/*37*/	(8 * 2),	//���� - 32*16 = 512ms
/*38*/	(8 * 2),	//8 - 32*16 = 256ms !!!!!!!!!!!!!!!!!!!!!!!!!
/*39*/	(8 * 2),	//���� - 32*16 = 512ms

/*40*/	(8 * 2),	//���� - 32*16 = 512ms
/*41*/	(8 * 2),	//�ر� - 32*16 = 512ms
/*42*/	(8 * 16),	//����(...) - 32*128 = 4096ms
/*43*/	(8 * 2),	//��� - 32*16 = 512ms
/*44*/	(8 * 2),	//���� - 32*16 = 512ms
/*45*/	(8 * 2),	//���� - 32*16 = 512ms
/*46*/	(8 * 2),	//���� - 32*16 = 512ms
/*47*/	(8 * 2),	//�Ϸ� - 32*16 = 512ms
/*48*/	(8 * 2),	//�·� - 32*16 = 512ms
/*49*/	(8 * 2),	//�밴 - 32*16 = 512ms

/*50*/	(8 * 3),	//��ʼ�� - 32*24 = 640ms
/*51*/	(8 * 3),	//��һ�� - 32*24 = 640ms
/*52*/	(8 * 3),	//����Ա - 32*24 = 640ms
/*53*/	(8 * 3),	//��һö - 32*24 = 640ms
/*54*/	(8 * 4),	//���̲��� - 32*32 = 1024ms
/*55*/	(8 * 4),	//ȫ������ - 32*32 = 1024ms
/*56*/	(8 * 4),	//�ָ����� - 32*32 = 1024ms
/*57*/	(8 * 6),	//Ϊ����İ�ȫ... - 32*48 = 1536ms
/*58*/	(8 * 6),	//ת�����ֿ��� - 32*48 = 1536ms
/*59*/	(8 * 9),	//�����ͣ��������� - 32*72 = 2304ms

/*60*/	(8 * 9),	//��ӭʹ�û�԰ָ���� - 32*72 = 2304ms
};

void promptDelay(int _delay)
{
	action_t action;
    action.actionType = CACT_VOPWAT;
    action.actionTime = _delay;
    actionQueueIn(&g_promptQueue, &action);
}

void beep(void)
{
	action_t action;
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
}

void bbeep(void)
{
	action_t action;
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
    
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
}

void bbbeep(void)
{
	action_t action;
	
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
   
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
    
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[CVOPID_BEEP] << 5);
    action.actionPara = CVOPID_BEEP;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
}

void vp_stor(unsigned char __vpIdx)
{
	action_t action;
    action.actionType = CACT_VOPON;
    //action.actionTime = TIMER_10SEC;
    action.actionTime = (u8VopLen32Time_tab[__vpIdx] << 5);
    action.actionPara = __vpIdx;
    action.func = (paction_t_0)vp_play;
    actionQueueIn(&g_promptQueue, &action);
}

volatile u8 g_tmr_sbuf;

#if	0
void vp_play(u8 __vpIdx)
{
	IRQ_disable();
	g_tmr_sbuf = __vpIdx;
	g_tmr_vop = 0;
	IRQ_enable();
}
#endif


/*******************************************************************************
 * ��������
 *******************************************************************************/
void lockAction(void)
{
	action_t action;
	/** ��ת���� **/
	actionQueueInit(&g_actionQueue, &(g_timer[1]), CACT_TOUT, CACT_OVER);
	stopAction(&g_actionQueue);
	
	#if	1
	action.actionType = CACT_MOTONORM;
    action.actionTime = TIMER_500MS;
    action.func = moto_normal;
    actionQueueIn(&g_actionQueue, &action);
    #endif
    
    /** ͣ�� **/
    action.actionType = CACT_MOTOWAIT;
    action.actionTime = TIMER_5SEC;
    action.func = moto_poweroff;
    actionQueueIn(&g_actionQueue, &action);
    
    #if	1
    action.actionType = CACT_MOTOREVE;
    action.actionTime = TIMER_500MS;
    action.func = moto_reverse;
    actionQueueIn(&g_actionQueue, &action);
    #endif
    
    action.actionType = CACT_MOTOWAIT;
    action.actionTime = TIMER_100MS;
    action.func = moto_poweroff;
    actionQueueIn(&g_actionQueue, &action);
}

/*****************************************************************************
 * function: arrSort()
 * Description: array sort
 * input: 	arr - array pointer
 			n - the number of element
 * output: no
 * return: no
****************************************************************************/
void arrSort(unsigned int arr[],unsigned char n)                         //��������
{
	unsigned char m,k,j,i;
	unsigned int d;
	k=0;
	m=n-1;
	while(k < m) {
		j = m - 1;
		m = 0;
		for(i = k; i <= j; i++) {
			if(arr[i] > arr[i + 1])	{
				d = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = d;
				m = i;
			}
		}
		j = k + 1;
		k = 0;
		for(i = m; i >= j; i--) {
			if(arr[i - 1] > arr[i]) {
				d = arr[i];
				arr[i] = arr[i - 1];
				arr[i - 1] = d;
				k = i;
			}
		}
	}
}
/*******************************************************************************
 * function: arrAverage
 * Description: calculate array average
 * input: 	arr - array pointer
 *			start - the start index of element
 *			end -   the end index of element
 * output: no
 * return: average
********************************************************************************/
unsigned int arrAverage(unsigned int arr[], unsigned char start, unsigned char end)       //����ƽ��ֵ
{
	int	i;
	unsigned int sum = 0;
	if(start > end) {
		return	0xffff;		/** error **/
	}
	for(i = start; i < end; i++) {
		sum += arr[i];
	}
	
	return	sum /(end - start +1);
}

#if	0
/*******************************************************************************
 * function: wdg_init
 * Description: 
 * input: no
 * output: no
 * return: no
********************************************************************************/
void wdg_init(void)
{

    /* IWDG timeout equal to 350ms (the timeout may varies due to LSI frequency
    dispersion) -------------------------------------------------------------*/
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
    IWDG_SetPrescaler(IWDG_Prescaler_32);

    /* Set counter reload value to 349 */
    IWDG_SetReload(349);
    //IWDG_SetReload(1999);

    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
}

void wdg_feed(void)
{
    /* Reload IWDG counter */
    IWDG_ReloadCounter();
}
#endif

#if	0
#endif

#if	0
#endif
////////////////////////////////////////////////////////////
