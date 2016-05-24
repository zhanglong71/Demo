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
 * 初始化时钟
 *******************************************************************************/
 void ClkInit(void)
 {
 	CLK_DeInit();			//复位时钟寄存器
  	CLK_HSICmd(ENABLE);		//使能内部高速时钟
  	//CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV4);//配置内部高速时钟(时钟分频器:4分频),系统时钟4MHz
  	CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);	//配置内部高速时钟(时钟分频器:1分频),系统时钟16MHz
  	CLK_ClockSecuritySystemEnable();				//开启时钟安全系统
}

/*******************************************************************************
 * 未使用的端口
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
/** 上电初始化外设 **/
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
    MSYSPOWER_INIT("端口初始化");
	FPort_Init();
	datetime_iic_port_init();
	datetimeDevice_init();
	//datetime_init();
    I2C_TOUCH_Init();
    EXIT_Init();
    OLED_SPI_Init();
    VOPPort_Init();
    VOP_Init();
	/** 开启低电采样 **/
	VoltageDetection_Init();
    MSYSPOWER_ON();		//开启低电采样信号源
    MFPOWER_OFF();		//关闭指纹传感器
	BLUETOOTHPort_Init();
}

/******************************************************************************
 * halt之前的准备工作：关闭设备
 ******************************************************************************/
void Halt_OffDevice(void)
{
	led_off(0);			//关闭led背光
    OLED_POWEROFF();	//关闭oled屏显电源
    datetime_Deinit();
    MFPOWER_OFF();		//关闭指纹传感器电源
    MSYSPOWER_OFF();	//关闭低电采样源
    bluetooth_PWRoff();	//关闭bluetooth
	//关闭设备前，设置系统主时钟和中断
	
	//ClkInit();
	SPI_Flash_PowerDown();
	ADC2_DeInit();		//STM8s207没有ADC1
	TIM1_DeInit();		//
	UART1_DeInit();		//STM8s207没有UART2, 有UART3
	
	/** Touch端口 **/
	I2C_TOUCH_DeInit();
	//停机前关闭不需要的功能模块的时钟
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI,DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART2,DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART3,DISABLE);	//没用到的也要关闭
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1, DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3, DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER5, DISABLE);	//没用到的也要关闭
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER6, DISABLE);	//没用到的也要关闭
	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC, DISABLE);
	//CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU,DISABLE);		//没用到的也要关闭	
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_CAN,DISABLE);		//没用到的也要关闭(stm8s207没这个)
	
	/** OLED显示用5个GPIO端口全是输出口，设置为无效片选状态 **/
	
	OLED_SPI_DeInit();
	/** voice语音用 **/
	VOPPort_DeInit();
	/** LED背光 **/
	LEDPort_DeInit();
	
	 //开启LSI
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
 * 唤醒后开启外设
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
    MSYSPOWER_ON();	//开启低电采样电源
    //EXIT_Init();
    
    VOPPort_Init();
    VOP_Init();
    
    OLED_SPI_Init();
    OLEDReset_IC();
    OLEDInit_IC();
	BLUETOOTHPort_Init();
}

/*******************************************************************************
 * 定时唤醒后发现蓝牙连接而开启的外设
 * 与外设唤醒不同的是：
 * 1.关闭按键背光(通用I/O)
 * 2.关闭OLED显示
 * 3.关闭指纹传感器电源
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
    MSYSPOWER_ON();	//开启低电采样电源
    //EXIT_Init();
    
    VOPPort_Init();
    VOP_Init();
    
    OLED_SPI_Init();
    OLEDReset_IC();
    OLEDInit_IC();
    
	All_Screen(0);		//关闭oled屏显
	//LEDPort_Init();
	led_off(0);		//关闭led背光
	MFPOWER_OFF();	//关闭指纹传感器电源
	//BLUETOOTHPort_Init();
}

/*******************************************************************************
 * 
 * 唤醒事件检测
 * 两种情况
 * 1.端口发生电平变化，则有外部事件发生
 * 2.无端口电平变化，认为是自动唤醒
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
 * 电压ADC检测
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
 * 电机控制
 *******************************************************************************/
void MOTOPort_Init(void)
{
  /*******
   * 电机
   *******/
   GPIO_Init(MOTO1_PORT,MOTO1_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );//定义开锁电机的管脚的模式
   GPIO_Init(MOTO2_PORT,MOTO2_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );
   
   moto_poweroff();
}

void MOTOPort_DeInit(void)
{
   GPIO_Init(MOTO1_PORT,MOTO1_PIN, GPIO_MODE_IN_FL_NO_IT );//定义开锁电机的管脚的模式
   GPIO_Init(MOTO2_PORT,MOTO2_PIN, GPIO_MODE_IN_FL_NO_IT );
}

void moto_normal(void) {GPIO_WriteLow(MOTO2_PORT, MOTO2_PIN);GPIO_WriteHigh(MOTO1_PORT, MOTO1_PIN);}
void moto_reverse(void) {GPIO_WriteHigh(MOTO2_PORT, MOTO2_PIN);GPIO_WriteLow(MOTO1_PORT, MOTO1_PIN);}
void moto_poweroff(void) {GPIO_WriteLow(MOTO2_PORT, MOTO2_PIN);GPIO_WriteLow(MOTO1_PORT, MOTO1_PIN);}

/*******************************************************************************
 * LED控制
 *******************************************************************************/
void LEDPort_Init(void)
{
	/**
   	 * LED背光
   	 **/ 
   #if	1
   GPIO_Init(LED1_PORT,LED1_PIN,GPIO_MODE_OUT_PP_LOW_SLOW );//定义LED的管脚的模式
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
   GPIO_Init(LED1_PORT,LED1_PIN,GPIO_MODE_OUT_PP_HIGH_SLOW );//定义LED的管脚的模式
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
   	 * LED背光
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
 * BLUETOOTH控制
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

//#define MBLUETOOTH_LINKCHK()  (GPIO_ReadInputPin(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN) != 0) //高有效
#define MBLUETOOTH_LINKCHK()  (GPIO_ReadInputPin(BLE_LINK_CHK_PORT, BLE_LINK_CHK_PIN) == 0) //低有效
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
/** 指示灯动作提示 **/
void ledPrompt(int ledNO, int _delay/** 周期 **/)
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

/** 指示灯警告提示 **/
void ledWarn(int _delay)	/** 警告提示 **/
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
 * 将指定的变量在[_max, _min]区间内加/减1. 
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
 * 将指定的变量在[99, 0]区间内加/减1. 
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
		if(*__press != 0) {			//手指靠近
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
	
	/** 检测按键状态的变化 **/
	if(KB_WAKECHECK(x)) {  //检测到按键
		
		if((g_flag & (1 << 7)) == (1 << 7))	{	//已经发过
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
		if(SET_WAKECHECK("按下设置键")) {
			g_tmr_setkey = TIMER_50MS;
			g_flag |= (1 << 12);
		}
	} else if(u8tmp == 1) {
		/** pressed counter **/
		if(SET_WAKECHECK("按下设置键")) {
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
		if(SET_WAKECHECK("按下设置键")) {
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
		if(SET_WAKECHECK("按下设置键")) {
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
	/** 按下指纹头 **/
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
 * 外设端口检测
 * GPIOB(1,2,3,4,5)
 * GPIOE(4)
 * 拼凑成状态码:        0b000x 0000 00xx xxx0
 * 状态掩码：	 0x103E(0b0001 0000 0011 1110)
 * 无事件状态码: 0x003A(0b0000 0000 0011 1010)   
 * 
 * 防撬事件码:   0x0038(0b0000 0000 0011 1000) - 0b0000 0000 0000 00x0
 * 遥控事件码:   0x003E(0b0000 0000 0011 1110) - 0b0000 0000 0000 0x00
 * 触摸事件码:   0x0032(0b0000 0000 0011 0010) - 0b0000 0000 0000 x000
 * 声音按键码:   0x002A(0b0000 0000 0010 1010) - 0b0000 0000 000x 0000
 * 设置按键码:   0x001A(0b0000 0000 0001 1010) - 0b0000 0000 00x0 0000
 * 指纹动作码:   0x103A(0b0001 0000 0011 1010) - 0b000x 0000 0000 0000
 * 
 * 目前用到的433/315模块接收到信号后，会发出2个500ms的高脉冲，间隔250ms. 最后返回到低电平状态
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
		//if(SET_WAKECHECK("按下设置键")) {
		if(MisINPUTBUSY) {
			g_tmr_setkey = TIMER_50MS;
			g_flag |= (1 << 12);
		}
	} else if(u8tmp == 1) {
		/** pressed counter **/
		//if(SET_WAKECHECK("按下设置键")) {
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
		//if(SET_WAKECHECK("按下设置键")) {
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
		//if(SET_WAKECHECK("按下设置键")) {
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
 * 检查输入脚是否有有效事件 
 *
 * 好像函数似乎没用到
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
 * 语音端口初始化：设置为推挽输出
 *******************************************************************************/
void VOPPort_Init(void)
{
	GPIO_Init(VOICE_DATA_PORT, VOICE_DATA_PIN, GPIO_MODE_OUT_PP_HIGH_FAST );//定义LED的管脚的模式
	GPIO_Init(VOICE_RST_PORT, VOICE_RST_PIN, GPIO_MODE_OUT_PP_HIGH_FAST );
}

/*********************************************************************
 * 进入低功耗状态。(这些端口设为浮空输入端口) 
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

/** 语音复位 **/
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
 * vop_busy()语音播放
 *	(CVOP_NY3P --- 一个由高(检到2次)到低(检到200次)的跳变，表示播放结束)
 *******************************************************************************/
void vop_busy(void)
{
#if	0
	//#define	CMAX_BUSY	100
	#define	CMAX_IDLE	20
	msg_t msg;
	static u8 u8Count = 0;
	
	//static u8 u8VopBusyFlag = 0;
	/** 查语音busy信号 H --> L
	 * 为防止错误检测及其它异常情况发生，采用以下特殊处理：
	 * 1. 一条语音指令开始后对g_tmr_iVopBusy清零, 直到自增至TIMER_VOPBUSY后才开始检测
	 * 2. g_tmr_iVopBusy自增至TIMER_VOPBUSY后查busy状态
	 * 3. busy持续指定时长后。置位
	 * 4. idle持续指定时长后。置位
	 *
	 * 到此：如果bit0, 3或bit1, 2有效则表示有电平变化
	 * 2，3同时有效，表示有稳定的电平变化
	 **/
	 
	if(g_tmr_iVopBusy == TIMER_VOPBUSY) {	/** 语音启动命令已经完成 **/
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
 * 动作处理: 启动新的动作
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
 * 动作处理: 启动新的动作
 *******************************************************************************/
static void startAction(actionQueue_t *q, action_t * pAction)
{
	SetTimer_irq(q->timer, pAction->actionTime, q->stepMsgType);
	
	switch(pAction->actionType)
	{
	/** 锁体转动(可以处理结束消息) **/	
    case    CACT_MOTONORM:	//正转
    case	CACT_MOTOREVE:	//反转
    case	CACT_MOTOWAIT:	//停转
		q->flag = (1<<7);
        if(pAction->func != 0)pAction->func();
		break;	
		
	/** 指示灯提示闪烁(在sys处理结束消息) **/
	case	CACT_LEDPMTOFF:
	case	CACT_LEDPMTON:
	case	CACT_LEDPMTWAT:
		q->flag = (1<<6);
        if(pAction->func != 0)pAction->func();
		break;	
		
	/** 指示灯警告闪烁(可以归入到提示闪烁中) **/		
	case	CACT_LEDWRNON:
	case	CACT_LEDWRNOFF:
	case	CACT_LEDWRNWAT:
		q->flag = (1<<5);
        if(pAction->func != 0)pAction->func();
		break;
		
	/** beep蜂鸣器(可以归入到提示闪烁中) **/
	case	CACT_BEEPON:
		q->flag = (1<<4);
		MVOPDATA_H();
		break;		
	case	CACT_BEEPOFF:
		q->flag = (1<<4);
		MVOPDATA_L();
		break;
		
	/** voiceprompt语音 **/
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
		
	/** misc其它杂项 **/	
	case	CACT_MISC:
		q->flag = (1<<1);
		if(pAction->func != 0)pAction->func();
		break;
			
	default:
		/** 不可识别的动作 **/
		q->flag = 0;
		break;
	}
}

/*******************************************************************************
 * 动作处理：停止当前的动作
 * 目前多是清除定时器，清除标记
 *
 * (忘记了可以这样处理的原因，什么时候想起，记得补上，是要立即开始下一个动作)
 *
 * 似乎应该在后面加上一个善后处理函数，以恢复到空闲状态。类似于面向对象里的析构函数
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
 /** 清空闪灯动作队列，并且停止当前动作. 只有上电时作用 **/
void blinkInit(void)
{
	actionQueueInit(&g_blinkQueue, &(g_timer[3]), CBLK_TOUT, CBLK_OVER);
	stopAction(&g_blinkQueue);
}

 /** 清空语音动作队列，并且停止当前语音动作 **/
void promptInit(void)
{
	actionQueueInit(&g_promptQueue, &(g_timer[2]), CPMT_TOUT, CPMT_OVER);
	stopAction(&g_promptQueue);
}

 /** 清空动作队列，并且停止当前动作 **/
void actionInit(void)
{
	actionQueueInit(&g_actionQueue, &(g_timer[1]), CACT_TOUT, CACT_OVER);
	stopAction(&g_actionQueue);
}
#endif

/** 事件队列处理 **/
void actionDoing(actionQueue_t * q)
{
	action_t action;
	
	if(q->flag == 0) {
		if(actionQueueOut(q, &action) == TRUE) {
			startAction(q, &action);
		}
	}
}

/** 系统当前事件结束 **/
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
 * 以8bit能表达最大8000，(13bit能表达的最大值8192)对8bit的变量需要左移5位
 * 即每单位表达32ms
 * 原则上，以字数为倍数，每字256ms(8)
 *******************************************************************************/
const u8 u8VopLen32Time_tab[] = {
/*0*/	0,	//空
/*1*/	1,	//silent
/*2*/	4,	//beep- 32*4 = 128ms
/*3*/	(8 * 1),	//号 - 32*8 = 256ms
/*4*/	(8 * 1),	//效 - 32*8 = 256ms
/*5*/	(8 * 1),	//无 - 32*8 = 256ms
/*6*/	(8 * 1),	//*  - 32*8 = 256ms
/*7*/	(8 * 1),	//#  - 32*8 = 256ms
/*8*/	(8 * 1),	//键 - 32*8 = 256ms
/*9*/	(8 * 1),	//已 - 32*8 = 256ms

/*10*/	(8 * 1),	//满 - 32*8 = 256ms
/*11*/	(8 * 1),	//按 - 32*8 = 256ms
/*12*/	(8 * 1),	//请 - 32*8 = 256ms
/*13*/	(8 * 1),	//再 - 32*8 = 256ms
/*14*/	(8 * 1),	//次 - 32*8 = 256ms
/*15*/	(8 * 2),	//密码 - 32*(8 * 2) = 512ms
/*16*/	(8 * 2),	//通过 - 32*16 = 512ms
/*17*/	(8 * 2),	//输入 - 32*16 = 512ms
/*18*/	(8 * 2),	//手指 - 32*16 = 512ms
/*19*/	(8 * 2),	//添加 - 32*16 = 512ms

/*20*/	(8 * 2),	//删除 - 32*16 = 512ms
/*21*/	(8 * 2),	//设置 - 32*16 = 512ms
/*22*/	(8 * 2),	//系统 - 32*16 = 512ms
/*23*/	(8 * 2),	//解锁 - 32*16 = 512ms
/*24*/	(8 * 2),	//验证 - 32*16 = 512ms
/*25*/	(8 * 2),	//成功 - 32*16 = 512ms
/*26*/	(8 * 2),	//失败 - 32*16 = 512ms
/*27*/	(8 * 2),	//指纹 - 32*16 = 512ms
/*28*/	(8 * 2),	//登录 - 32*16 = 512ms
/*29*/	(8 * 2),	//正在 - 32*16 = 512ms !!!!!!!!!!!!!!!!!!!!!!!!!

/*30*/	(8 * 2),	//稍候 - 32*16 = 512ms
/*31*/	(8 * 2),	//清空 - 32*16 = 512ms
/*32*/	(8 * 1),	//2 - 32*8 = 256ms
/*33*/	(8 * 2),	//功能 - 32*16 = 512ms
/*34*/	(8 * 2),	//确认 - 32*16 = 512ms
/*35*/	(8 * 2),	//取消 - 32*16 = 512ms
/*36*/	(8 * 2),	//用户 - 32*16 = 512ms
/*37*/	(8 * 2),	//所有 - 32*16 = 512ms
/*38*/	(8 * 2),	//8 - 32*16 = 256ms !!!!!!!!!!!!!!!!!!!!!!!!!
/*39*/	(8 * 2),	//语音 - 32*16 = 512ms

/*40*/	(8 * 2),	//开启 - 32*16 = 512ms
/*41*/	(8 * 2),	//关闭 - 32*16 = 512ms
/*42*/	(8 * 16),	//报警(...) - 32*128 = 4096ms
/*43*/	(8 * 2),	//编号 - 32*16 = 512ms
/*44*/	(8 * 2),	//锁定 - 32*16 = 512ms
/*45*/	(8 * 2),	//结束 - 32*16 = 512ms
/*46*/	(8 * 2),	//返回 - 32*16 = 512ms
/*47*/	(8 * 2),	//上翻 - 32*16 = 512ms
/*48*/	(8 * 2),	//下翻 - 32*16 = 512ms
/*49*/	(8 * 2),	//请按 - 32*16 = 512ms

/*50*/	(8 * 3),	//初始化 - 32*24 = 640ms
/*51*/	(8 * 3),	//不一致 - 32*24 = 640ms
/*52*/	(8 * 3),	//管理员 - 32*24 = 640ms
/*53*/	(8 * 3),	//另一枚 - 32*24 = 640ms
/*54*/	(8 * 4),	//续继操作 - 32*32 = 1024ms
/*55*/	(8 * 4),	//全部数据 - 32*32 = 1024ms
/*56*/	(8 * 4),	//恢复出厂 - 32*32 = 1024ms
/*57*/	(8 * 6),	//为了你的安全... - 32*48 = 1536ms
/*58*/	(8 * 6),	//转动把手开门 - 32*48 = 1536ms
/*59*/	(8 * 9),	//电量低，请更换电池 - 32*72 = 2304ms

/*60*/	(8 * 9),	//欢迎使用华园指纹锁 - 32*72 = 2304ms
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
 * 开锁操作
 *******************************************************************************/
void lockAction(void)
{
	action_t action;
	/** 正转过程 **/
	actionQueueInit(&g_actionQueue, &(g_timer[1]), CACT_TOUT, CACT_OVER);
	stopAction(&g_actionQueue);
	
	#if	1
	action.actionType = CACT_MOTONORM;
    action.actionTime = TIMER_500MS;
    action.func = moto_normal;
    actionQueueIn(&g_actionQueue, &action);
    #endif
    
    /** 停顿 **/
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
void arrSort(unsigned int arr[],unsigned char n)                         //数组排序
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
unsigned int arrAverage(unsigned int arr[], unsigned char start, unsigned char end)       //数组平均值
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
