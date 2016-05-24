#ifndef  __EXTI_H
#define  __EXTI_H
#include "stm8s.h"

/** GPIOE.4 **/
#define FP_WAKECHECK_PIN        GPIO_PIN_4
#define FP_WAKECHECK_PORT       GPIOE

/** GPIOB.5(SET_IRQ) **/
#define SET_IRQ_PIN        GPIO_PIN_5
#define SET_IRQ_PORT       GPIOB

/** GPIOB.4(MUSIC_IRQ) **/
#define MUSIC_IRQ_PIN        GPIO_PIN_4
#define MUSIC_IRQ_PORT       GPIOB

/** GPIOB.3 **/
#define TOUCH_IRQ_PIN        GPIO_PIN_3
#define TOUCH_IRQ_PORT       GPIOB

/** GPIOB.2(YK_IRQ) **/
#define REMOTE_IRQ_PIN        GPIO_PIN_2
#define REMOTE_IRQ_PORT       GPIOB

/** GPIOB.1 **/
#define FANGQIAO_IRQ_PIN        GPIO_PIN_1
#define FANGQIAO_IRQ_PORT       GPIOB

/******************************************/
#define FP_WAKECHECK(x)  (GPIO_ReadInputPin(FP_WAKECHECK_PORT, FP_WAKECHECK_PIN) == 1) 	//高有效
#define KB_WAKECHECK(x)  (GPIO_ReadInputPin(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN) == 0)   		//低有效
#define SET_WAKECHECK(x)  (GPIO_ReadInputPin(SET_IRQ_PORT, SET_IRQ_PIN) == 0)   		//低有效
#define MUSIC_WAKECHECK(x)  (GPIO_ReadInputPin(MUSIC_IRQ_PORT, MUSIC_IRQ_PIN) == 0)   	//低有效
#define REMOTE_WAKECHECK(x)  (GPIO_ReadInputPin(REMOTE_IRQ_PORT, REMOTE_IRQ_PIN) == 1)  //高有效

#define FANGQIAO_WAKECHECK(x)  (GPIO_ReadInputPin(MUSIC_IRQ_PORT, MUSIC_IRQ_PIN) == 0)  //低有效
/******************************************/
//void EXIT_KEY_Init(void);
void EXIT_Init(void);
void EXIT_process(void);

#endif