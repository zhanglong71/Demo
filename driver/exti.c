/******************** (C) COPYRIGHT 2012 青风电子 ********************
 * 文件名  ：main
 * 描述    ：         
 * 实验平台：青风stm8发板
 * 描述    ：按键中断
 * 作者    ：青风
 * 店铺    ：qfv5.taobao.com
  硬件：FP/SET/touch/YK/remote/Fangqiao
**********************************************************************/
#include "exti.h"
#include "stm8s_gpio.h"
#include "stm8s_exti.h"

/*******************************************************************************
 * 外部中断相关
 * 五个地方：
 * 	1. 指纹感应(高有效，需要浮空输入，外置下拉)
 * 	2. 遥控器接口(高有效，需要浮空输入，外置下拉)
 *
 * 	3. 设置键(低有效，可置内部上拉)
 * 	4. 触摸面板感应(低有效，可置内部上拉)
 * 	5. 防撬接口(低有效，可置内部上拉)
 * 	6. VOP接口(低有效，可置内部上拉)
 *******************************************************************************/
void EXIT_Init(void)
{
   GPIO_Init(FP_WAKECHECK_PORT, FP_WAKECHECK_PIN, GPIO_MODE_IN_FL_IT );	//FP_wakecheck
   GPIO_Init(REMOTE_IRQ_PORT, REMOTE_IRQ_PIN, GPIO_MODE_IN_FL_IT );		//remote
   //GPIO_Init(REMOTE_IRQ_PORT, REMOTE_IRQ_PIN, GPIO_MODE_IN_PU_IT );	//remote
   
   GPIO_Init(SET_IRQ_PORT, SET_IRQ_PIN, GPIO_MODE_IN_PU_IT );			//set
   GPIO_Init(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN, GPIO_MODE_IN_PU_IT );		//touch
   GPIO_Init(FANGQIAO_IRQ_PORT, FANGQIAO_IRQ_PIN, GPIO_MODE_IN_PU_IT );	//Fangqiao
   GPIO_Init(MUSIC_IRQ_PORT, MUSIC_IRQ_PIN, GPIO_MODE_IN_PU_IT );		//VOP
   
   //GPIO_Init(KEY1_PORT,KEY1_PIN,GPIO_MODE_IN_PU_IT );//定义key的管脚的模式
   //GPIO_Init(KEY2_PORT,KEY2_PIN,GPIO_MODE_IN_PU_IT );//区别与之前的按键扫描IO的设置
   //GPIO_Init(KEY3_PORT,KEY3_PIN,GPIO_MODE_IN_PU_IT );
   //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_RISE_ONLY);//外部中断触发方式
   //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);//外部中断触发方式
   
   EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_RISE_FALL);//外部中断触发方式
   EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_FALL);//外部中断触发方式
}

void EXIT_process(void)
{
  
}