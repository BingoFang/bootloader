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
}cmd_list_t;

/* 状态类型定义 */
typedef enum 
{
	STATUS_OK 					= 0x00U,
	STATUS_ERROR				= 0x01U,
	STATUS_JUMP_FAIL 		= 0x02U,
	STATUS_CRC_FAIL			= 0x03U,
	STATUS_RECV_TIMEOUT = 0x04U 
}status_type_t;


void JumpFirmwareSuccess(void);
uint8_t CAN_BOOT_GetAddrData(void);
void CAN_BOOT_ExecutiveCommand(CanRxMsg *pRxMessage);
void HandleCanQueue(void);

#endif

