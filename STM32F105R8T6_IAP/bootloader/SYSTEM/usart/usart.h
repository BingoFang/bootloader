#ifndef __USART_H__
#define __USART_H__
#include "stdio.h"	
#include "sys.h" 


void USART1_Init(u32 bound);
void USART1_SendData(u8 *buf, u8 len);

#endif


