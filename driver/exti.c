/******************** (C) COPYRIGHT 2012 ������ ********************
 * �ļ���  ��main
 * ����    ��         
 * ʵ��ƽ̨�����stm8����
 * ����    �������ж�
 * ����    �����
 * ����    ��qfv5.taobao.com
  Ӳ����FP/SET/touch/YK/remote/Fangqiao
**********************************************************************/
#include "exti.h"
#include "stm8s_gpio.h"
#include "stm8s_exti.h"

/*******************************************************************************
 * �ⲿ�ж����
 * ����ط���
 * 	1. ָ�Ƹ�Ӧ(����Ч����Ҫ�������룬��������)
 * 	2. ң�����ӿ�(����Ч����Ҫ�������룬��������)
 *
 * 	3. ���ü�(����Ч�������ڲ�����)
 * 	4. ��������Ӧ(����Ч�������ڲ�����)
 * 	5. ���˽ӿ�(����Ч�������ڲ�����)
 * 	6. VOP�ӿ�(����Ч�������ڲ�����)
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
   
   //GPIO_Init(KEY1_PORT,KEY1_PIN,GPIO_MODE_IN_PU_IT );//����key�Ĺܽŵ�ģʽ
   //GPIO_Init(KEY2_PORT,KEY2_PIN,GPIO_MODE_IN_PU_IT );//������֮ǰ�İ���ɨ��IO������
   //GPIO_Init(KEY3_PORT,KEY3_PIN,GPIO_MODE_IN_PU_IT );
   //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_RISE_ONLY);//�ⲿ�жϴ�����ʽ
   //EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_FALL_ONLY);//�ⲿ�жϴ�����ʽ
   
   EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOE, EXTI_SENSITIVITY_RISE_FALL);//�ⲿ�жϴ�����ʽ
   EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_FALL);//�ⲿ�жϴ�����ʽ
}

void EXIT_process(void)
{
  
}