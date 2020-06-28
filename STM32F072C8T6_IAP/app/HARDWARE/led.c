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
�������ܣ�LED1����
�����������
�����������
��    ע�����ô˺���ǰ����Ҫ��LED.h�޸ĺ궨��LED����
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
�������ܣ�LED2����
�����������
�����������
��    ע�����ô˺���ǰ����Ҫ��LED.h�޸ĺ궨��LED����
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
�������ܣ�LED3����
�����������
�����������
��    ע�����ô˺���ǰ����Ҫ��LED.h�޸ĺ궨��LED����
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
�������ܣ�LED4����
�����������
�����������
��    ע�����ô˺���ǰ����Ҫ��LED.h�޸ĺ궨��LED����
****************************************************/
void LED4_ON(void)
{
		GPIO_ResetBits(LED4_PORT, LED4_PIN);
}

void LED4_OFF(void)
{
		GPIO_SetBits(LED4_PORT, LED4_PIN);
}



