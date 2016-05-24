#include "uart.h"
#include <stdarg.h>
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
/* Private functions ---------------------------------------------------------*/

void USART_Configuration(void)//串口初始化函数
{  
/** UART1**/
    UART1_DeInit();
    //UART1_Init((u32)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE ,UART1_MODE_TXRX_ENABLE);//波特率，字节数，1个停止位，无奇偶效验位，非同步模式，允许接受和发送
    UART1_Init((u32)115200, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    //UART1_Init((u32)115200, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO, UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_RX_ENABLE);

	//UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);		//!!!!!!!!!!!
	UART1_ITConfig(UART1_IT_RXNE, ENABLE);		//!!!!!!!!!!!
    UART1_Cmd(ENABLE );
    
/** UART3 **/ 
    #if	1
    UART3_DeInit();
    UART3_Init((u32)9600, UART3_WORDLENGTH_8D, UART3_STOPBITS_1, UART3_PARITY_NO, UART3_MODE_TXRX_ENABLE);	//波特率，字节数，1个停止位，无奇偶效验位，非同步模式，允许接受和发送
	UART3_ITConfig(UART3_IT_RXNE_OR, ENABLE);		//!!!!!!!!!!!
    UART3_Cmd(ENABLE );
    #endif
}			

/** UART1 **/
void UART_send_byte(uint8_t byte) //发送1字节数据
{
    UART1_SendData8((unsigned char)byte);
	/* Loop until the end of transmission */
   	while(UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);//发送数据为空
}		

void UART_Send(uint8_t *Buffer, uint16_t Length)
{
	uint16_t l = 0;
	for(; l < Length; l++) {
    	UART_send_byte(Buffer[l]);
	}
}

uint8_t UART_Recive(void)
{	
	uint8_t UART1_Re_Buf; 
    while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);//接受数据寄存器非空
    UART1_Re_Buf = UART1_ReceiveData8();
    return  UART1_Re_Buf;
}

/** UART3 **/
void UART3_send_byte(uint8_t byte) //发送1字节数据
{
    UART3_SendData8((unsigned char)byte);
	/* Loop until the end of transmission */
   	while(UART3_GetFlagStatus(UART3_FLAG_TXE) == RESET);//发送数据为空
}		

void UART3_Send(uint8_t *Buffer, uint16_t Length)
{
	uint16_t l = 0;
	for(; l < Length; l++) {
		UART3_send_byte(Buffer[l]);
	}
}

uint8_t UART3_Recive(void)
{	
	uint8_t UART3_Re_Buf; 
	while(UART3_GetFlagStatus(UART3_FLAG_RXNE) == RESET);//接受数据寄存器非空
	UART3_Re_Buf = UART3_ReceiveData8();
	return  UART3_Re_Buf;
}

#if 0
PUTCHAR_PROTOTYPE ////发送一个字符协议
{
/* 将Printf内容发往串口 */
  UART1_SendData8((unsigned char) ch);
  while (!(UART1->SR & UART1_FLAG_TXE));//如果发送未完成， //标志位未置位，则循环等待
  return (ch);

}
#endif	
////////////////////////////////////////////////////////////////////////////////

