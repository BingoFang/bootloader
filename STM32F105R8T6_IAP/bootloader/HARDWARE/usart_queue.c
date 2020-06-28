#include "main.h"


usart_queue_t usart1_send;


void UsartQueueInit(usart_queue_t *q)
{
  q->size = 0;
  q->front = 0;
  q->rear = 0;
}

 
uint8_t UsartQueuePush(usart_queue_t *q, uint8_t data)
{
  if(((q->rear % USART_QUEUE_SIZE) == q->front) && (q->size == USART_QUEUE_SIZE))
  {
    return USART_QUEUE_FULL;
  }
 
  q->data[q->rear] = data;

  q->rear = (q->rear + 1) % USART_QUEUE_SIZE;

  q->size++;

  return USART_QUEUE_OK;
}

 

uint8_t UsartQueuePop(usart_queue_t *q, uint8_t *data)
{
  if((q->front == q->rear) && (q->size == 0))
  {
    return USART_QUEUE_EMPTY;
  }

  *data = q->data[q->front];

  q->front = (q->front + 1) % USART_QUEUE_SIZE;
 
  q->size--;

  return USART_QUEUE_OK;
}
