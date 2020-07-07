#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 


void USART1_Init(u32 bound);
void USART1_SendData(u8 *buf, u8 len);

#endif


