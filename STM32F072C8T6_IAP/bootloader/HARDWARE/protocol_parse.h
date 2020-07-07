#ifndef __PROTOCOL_PARSE_H__
#define __PROTOCOL_PARSE_H__

#include "stm32f0xx.h"

typedef struct
{
	uint8_t write_info;
	uint8_t write_bin;
	uint8_t check_version;
	uint8_t set_baundrate;
	uint8_t excute;
	uint8_t cmd_success;
	uint8_t cmd_failed;
}cmd_list_t;

void JumpFirmwareSuccess(void);
uint8_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void HandleCanQueue(void);

#endif

