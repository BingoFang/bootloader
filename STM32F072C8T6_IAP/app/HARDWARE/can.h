#ifndef __CAN_H__
#define __CAN_H__

#include "stm32f0xx.h"

typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

void CAN_Configuration(uint32_t BaudRate);
uint8_t CAN_WriteData(CanTxMsg *TxMessage);
void BOOT_DelayMs(uint32_t ms);
uint16_t Read_CAN_Address(void);
TestStatus CAN_Polling(void);
TestStatus CAN_Interrupt(void);

#endif

