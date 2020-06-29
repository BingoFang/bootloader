#ifndef __USART_QUEUE_H__
#define __USART_QUEUE_H__

#include "sys.h"


typedef enum
{
  USART_QUEUE_EMPTY = 0,
  USART_QUEUE_FULL = 1,
  USART_QUEUE_OK = 2,
} usart_queue_status_t;

 
#define USART_QUEUE_SIZE  256

typedef struct
{  
  uint16_t front;
  uint16_t rear;
  uint16_t size;
  char data[USART_QUEUE_SIZE];
} usart_queue_t;

 

extern usart_queue_t usart1_send;

 
void UsartQueueInit(usart_queue_t *q);
uint8_t UsartQueuePush(usart_queue_t *q, uint8_t data);
uint8_t UsartQueuePop(usart_queue_t *q, uint8_t *data);

#endif


