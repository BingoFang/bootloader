#ifndef __LED_H
#define __LED_H

#include "stm32f0xx.h"

#define LED_GPIO_CLK   		RCC_AHBPeriph_GPIOA 
#define LED1_PORT   	   	GPIOA
#define LED1_PIN        	GPIO_Pin_2

#define LED2_PORT   	   	GPIOA
#define LED2_PIN        	GPIO_Pin_3

#define LED3_PORT   	   	GPIOA
#define LED3_PIN        	GPIO_Pin_6

#define LED4_PORT   	   	GPIOA
#define LED4_PIN        	GPIO_Pin_7

void LED_Init(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED1_Toggle(void);
void LED2_ON(void);
void LED2_OFF(void);
void LED3_ON(void);
void LED3_OFF(void);
void LED4_ON(void);
void LED4_OFF(void);
#endif
