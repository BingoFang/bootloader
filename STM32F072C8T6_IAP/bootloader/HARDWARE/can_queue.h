#ifndef __CAN_QUEUE_H__
#define __CAN_QUEUE_H__
 
#include "stm32f0xx.h"
 

#define CAN_TX_SIZE      0x25


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum
{
  CAN_ERROR = 0,
  CAN_OK    = 1,
  CAN_BUSY  = 2,
} can_queue_status_t;


typedef CanRxMsg  can_frame_t;

typedef struct
{
  uint32_t write;
  uint32_t read;
  uint8_t is_full;
  uint32_t length;
  can_frame_t *frame;
} can_queue_t;

extern can_queue_t can_queue_send;

 
void CanQueueInit(void);
uint8_t CanQueueRead(can_queue_t *q, can_frame_t *frame);
uint8_t CanQueueWrite(can_queue_t *q, can_frame_t *frame);


#endif 

