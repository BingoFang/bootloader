#include "main.h"
#include "string.h"
 
static can_frame_t tx[CAN_TX_SIZE] = {0};

can_queue_t can_queue_send =
{
  0,
  0,
  FALSE,
  CAN_TX_SIZE,
  tx
};


const can_frame_t frame_init =
{
  0,
  0,
  0,
  0,
  0,
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

 
static void can_queue_init(can_queue_t *q, can_frame_t *frame, uint8_t length)
{
  uint8_t i;

  q->write = 0;
  q->read  = 0;
  q->is_full = FALSE;
  q->length = length;
  q->frame = frame;

  for(i = 0; i < length; ++i)
  {
    q->frame[i] = frame_init;
  }
}

 
static can_frame_t *can_frame_malloc(can_queue_t *q)
{
  can_frame_t *frame;

  if(NULL == q)
  {
    return NULL;
  }

  if(TRUE == q->is_full)
  {
    return NULL;
  }

  frame = &(q->frame[q->write++]);

  if(q->write >= q->length)
  {
    q->write = 0;
  }

  if(q->write == q->read)
  {
    q->is_full = TRUE;
  }

  *frame = frame_init;

  return frame;
}

 
static void can_frame_clear(can_frame_t *frame)
{
  can_frame_t *dst = frame;
  dst->StdId = 0;
  dst->DLC = 0;
  dst->IDE = CAN_Id_Extended;
  dst->RTR = CAN_RTR_Data;
  memset(dst->Data, 0, 8);
}

 
static void can_frame_copy(can_frame_t *dst, can_frame_t *src)
{
  can_frame_t *dst_tmp = dst, *src_tmp = src;
  dst_tmp->ExtId = src_tmp->ExtId;
  dst_tmp->DLC = src_tmp->DLC;
  dst_tmp->IDE = src_tmp->IDE;
  dst_tmp->RTR = src_tmp->RTR;
  memcpy(dst_tmp->Data, src_tmp->Data, 8);
}

 
uint8_t CanQueueRead(can_queue_t *q, can_frame_t *frame)
{
  uint8_t status = CAN_ERROR;

  if(NULL == q)
  {
    return CAN_ERROR;
  }

  if((q->read != q->write) || (q->is_full == TRUE))
  {
    can_frame_copy(frame, &q->frame[q->read]);
    can_frame_clear(&q->frame[q->read]);
    q->read += 1;

    if(q->read >= q->length)
    {
      q->read = 0;
    }
    q->is_full = FALSE;

		status = CAN_OK;
  }
  return status;
}

 
uint8_t CanQueueWrite(can_queue_t *q, can_frame_t *frame)
{
  can_frame_t *dst;
  dst = can_frame_malloc(q);

  if(NULL == dst)
  {
    return CAN_ERROR;
  }

  can_frame_copy(dst, frame);

  return CAN_OK;
}

 
void CanQueueInit(void)
{
  can_queue_init(&can_queue_send, tx, CAN_TX_SIZE);
}
