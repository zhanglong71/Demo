#ifndef __DEBUG_H__
#define __DEBUG_H__
/*******************************************************************************/
/** only for test **/
#if	1
	#define	reversion_GPIOA7()	do{\
									static unsigned char tmp = 0;	\
									if(tmp++ & 1)	\
									{	\
										GPIO_SetBits(GPIOA, GPIO_Pin_7);	\
									}	\
									else	\
									{	\
										GPIO_ResetBits(GPIOA, GPIO_Pin_7);	\
									}	\
								}while(0)

#endif


#if	1
	#define	output_GPIO7(x) do{\
									if(x)	\
									{	\
										GPIO_SetBits(GPIOA, GPIO_Pin_7);	\
									}	\
									else	\
									{	\
										GPIO_ResetBits(GPIOA, GPIO_Pin_7);	\
									}	\
							}while(0)

#endif

/**********************************************/
/* 函数功能；简单的延迟函数                   */
/* 入口参数：无                               */
/**********************************************/
#if	0
void delay()
{
	int i,j;
  for(i=0;i<1000;i++)
	  {
   for(j=0;j<1000;j++);
	}
}
#endif


#if 0
/** for test only**/
#define LEDtest_PIN        GPIO_PIN_7 
#define LEDtest_PORT       GPIOF

void LEDtest_off(void) {GPIO_WriteLow(LEDtest_PORT, LEDtest_PIN);}
void LEDtest_on(void) {GPIO_WriteHigh(LEDtest_PORT, LEDtest_PIN);}
void LEDtest_Toggle(void) {GPIO_WriteReverse(LEDtest_PORT,LEDtest_PIN);}
#endif
/*******************************************************************************/
#endif
