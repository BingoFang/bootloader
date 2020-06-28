#include "LED.h"

void LED_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;

	  RCC_AHBPeriphClockCmd(LED_GPIO_CLK, ENABLE);
   
    GPIO_InitStructure.GPIO_Pin = LED1_PIN|LED2_PIN|LED3_PIN|LED4_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
    GPIO_Init(LED1_PORT, &GPIO_InitStructure);
	
		LED1_OFF(); LED2_OFF(); LED3_OFF();	LED4_OFF();
}

/****************************************************
函数功能：LED1开关
输入参数：无
输出参数：无
备    注：调用此函数前，需要在LED.h修改宏定义LED引脚
****************************************************/
void LED1_ON(void)
{
		GPIO_ResetBits(LED1_PORT, LED1_PIN);
}

void LED1_OFF(void)
{
		GPIO_SetBits(LED1_PORT, LED1_PIN);
}

void LED1_Toggle(void)
{
		LED1_PORT->ODR ^= LED1_PIN; 
}

/****************************************************
函数功能：LED2开关
输入参数：无
输出参数：无
备    注：调用此函数前，需要在LED.h修改宏定义LED引脚
****************************************************/
void LED2_ON(void)
{
		GPIO_ResetBits(LED2_PORT, LED2_PIN);
}

void LED2_OFF(void)
{
		GPIO_SetBits(LED2_PORT, LED2_PIN);
}

/****************************************************
函数功能：LED3开关
输入参数：无
输出参数：无
备    注：调用此函数前，需要在LED.h修改宏定义LED引脚
****************************************************/
void LED3_ON(void)
{
		GPIO_ResetBits(LED3_PORT, LED3_PIN);
}

void LED3_OFF(void)
{
		GPIO_SetBits(LED3_PORT, LED3_PIN);
}

/****************************************************
函数功能：LED4开关
输入参数：无
输出参数：无
备    注：调用此函数前，需要在LED.h修改宏定义LED引脚
****************************************************/
void LED4_ON(void)
{
		GPIO_ResetBits(LED4_PORT, LED4_PIN);
}

void LED4_OFF(void)
{
		GPIO_SetBits(LED4_PORT, LED4_PIN);
}



