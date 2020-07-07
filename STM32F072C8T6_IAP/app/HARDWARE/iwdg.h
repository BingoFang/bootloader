#ifndef __IWDG_H__
#define __IWDG_H__

#include "stm32f0xx.h"

void IWDG_Init(uint8_t prer,uint16_t rlr);
void IWDG_Feed(void);


#endif

