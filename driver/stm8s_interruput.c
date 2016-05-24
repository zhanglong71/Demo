

#include "stm8s.h"
#include "CONST.h"
#include "ctype.h" 
#include "macro.h" 
#include "global.h" 

#include "stm8s_awu.h"
#include "stm8s_tim1.h"
#include "tim1.h"

#include "stm8s_uart1.h"
#include "stm8s_uart3.h"
#include "uart.h"	

#pragma vector=1
__interrupt void TRAP_IRQHandler(void)
{
  
}
#pragma vector=2
__interrupt void TLI_IRQHandler(void)
{
  
}
#pragma vector=3
__interrupt void AWU_IRQHandler(void)
{
	AWU_GetFlagStatus();
    
    g_IT_flag |= (1 << 4);
}
#pragma vector=4
__interrupt void CLK_IRQHandler(void)
{
  
  
}
#pragma vector=5
__interrupt void EXTI_PORTA_IRQHandler(void)
{
  
}
#pragma vector=6
__interrupt void EXTI_PORTB_IRQHandler(void)
{
  
}
#pragma vector=7
__interrupt void EXTI_PORTC_IRQHandler(void)
{
  
}
#pragma vector=8
__interrupt void EXTI_PORTD_IRQHandler(void)
{
  
}
#pragma vector=9
__interrupt void EXTI_PORTE_IRQHandler(void)
{
	/** 工作状态时发生的指纹传感器产生中断，可在查端口状态时滤掉 **/
	g_IT_flag |= (1 << 5);	/** only FingerPrint EXIT-Irq used GPIOE **/
}
#ifdef STM8S903
#pragma vector=0xA
__interrupt void EXTI_PORTF_IRQHandler(void)
{
  
}
#endif
#ifdef STM8S208
#pragma vector=0xA
__interrupt void CAN_RX_IRQHandler(void)
{
  
}
#pragma vector=0xB
__interrupt void CAN_TX_IRQHandler(void)
{
  
}
#endif
#pragma vector=0xC
__interrupt void SPI_IRQHandler(void)
{
  
}
#pragma vector=0xD
__interrupt void TIM1_UPD_OVF_TRG_BRK_IRQHandler(void)
{
  TimingDelay_Decrement();
  TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
  
}
#pragma vector=0xE
__interrupt void TIM1_CAP_COM_IRQHandler(void)
{
  
}
#ifdef STM8S903
#pragma vector=0xF
__interrupt void TIM5_UPD_OVF_BRK_TRG_IRQHandler(void)
{
  
}
#pragma vector=0x10
__interrupt void TIM5_CAP_COM_IRQHandler(void)
{
  
}
#else
#pragma vector=0xF
__interrupt void TIM2_UPD_OVF_BRK_IRQHandler(void)
{
     
}
#pragma vector=0x10
__interrupt void TIM2_CAP_COM_IRQHandler(void)
{
  
}
#endif
#if defined (STM8S208) || defined(STM8S207) || defined(STM8S105)
#pragma vector=0x11
__interrupt void TIM3_UPD_OVF_BRK_IRQHandler(void)
{
  
}
#pragma vector=0x12
__interrupt void TIM3_CAP_COM_IRQHandler(void)
{
  
}
#endif
//#ifndef STM8S105
#if defined (STM8S208) || defined(STM8S207) || defined(STM8S105)
#pragma vector=0x13
__interrupt void UART1_TX_IRQHandler(void)
{
  
}
#pragma vector=0x14
__interrupt void UART1_RX_IRQHandler(void)
{ 
	/*******************************************************/
	//uint8_t UART1_Re_Buf; 
	if((SET == UART1_GetITStatus(UART1_IT_RXNE))
		 || (SET == UART1_GetITStatus(UART1_IT_OR))) {
     	/** 接受数据寄存器非空 **/
     	if(UART1_GetFlagStatus(UART1_FLAG_RXNE) == SET) {
     		g_T_UART1Busy = 0;
     		//UART1_Re_Buf = UART1_ReceiveData8();
     		MFPACK_FIFO_IN(UART1_ReceiveData8());
     		
		    UART1_ClearITPendingBit(UART1_IT_RXNE);	
     	}

     	if(UART1_GetFlagStatus(UART1_FLAG_OR) == SET) {
     		g_T_UART1Busy = 0;
     		//UART1_Re_Buf = UART1_ReceiveData8();
     		MFPACK_FIFO_IN(UART1_ReceiveData8());
     		
		    UART1_ClearITPendingBit(UART1_IT_OR);	
     	}	
	} else if(SET == UART1_GetITStatus(UART1_IT_LBDF)) {	
	//} else if(SET == UART1_GetITStatus(UART1_IT_OR)) {
		UART1_ClearITPendingBit(UART1_IT_LBDF);
	} else {
		//UART1_ClearITPendingBit(UART1_IT_LBDF);
		//UART1_SendData8(0); //!!!!!!!!!!!!!!!!!!error reset		
    }
	/*******************************************************/
}
#endif
#pragma vector=0x15
__interrupt void I2C_IRQHandler(void)
{
  
}
#ifdef STM8S105
#pragma vector=0x16
__interrupt void UART2_TX_IRQHandler(void)
{
   
}
#pragma vector=0x17
__interrupt void UART2_RX_IRQHandler(void)
{
   
}
#endif
#if defined(STM8S207) || defined(STM8S208)
#pragma vector=0x16
__interrupt void UART3_TX_IRQHandler(void)
{
  
}
#pragma vector=0x17
__interrupt void UART3_RX_IRQHandler(void)
{
	/*******************************************************/
	if(SET == UART3_GetITStatus(UART3_IT_RXNE)) {
     	/** 接受数据寄存器非空 **/
     	if(UART3_GetFlagStatus(UART3_FLAG_RXNE) == SET) {
     		g_T_UART3Busy = 0;
     		MBTACK_FIFO_IN(UART3_ReceiveData8());
     	}

		UART3_ClearITPendingBit(UART3_IT_RXNE);		
        nop();
	} else if(SET == UART3_GetITStatus(UART3_IT_OR)) {
		UART3_ClearITPendingBit(UART3_IT_LBDF);
	} 
	/*******************************************************/
}
#endif
#if defined(STM8S207) || defined(STM8S208)
#pragma vector=0x18
__interrupt void ADC2_IRQHandler(void)
{
   
}
#else
#pragma vector=0x18
__interrupt void ADC1_IRQHandler(void)
{
   
}
#endif
#ifdef STM8S903
#pragma vector=0x19
__interrupt void TIM6_UPD_OVF_TRG_IRQHandler(void)
{
  
}
#else
#pragma vector=0x19
__interrupt void TIM4_UPD_OVF_IRQHandler(void)
{
  
}
#endif
#pragma vector=0x1A
__interrupt void EEPROM_EEC_IRQHandler(void)
{
  
}

