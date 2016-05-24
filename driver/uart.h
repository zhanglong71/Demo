#ifndef __UART_H
#define	__UART_H

#include "stm8s.h"
#include "stm8s_clk.h"
#include <stdio.h>

#include "stm8s_uart3.h"

void USART_Configuration(void);
void UART_send_byte(uint8_t byte);
void UART_Send(uint8_t *Buffer, uint16_t Length);
uint8_t UART_Recive(void);
//int fputc(int ch, FILE *f);

#endif /* __UART_H */
