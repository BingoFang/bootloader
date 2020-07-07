#ifndef __PROTOCOL_PARSE_H__
#define __PROTOCOL_PARSE_H__

#include "sys.h"

typedef enum
{
	UART_PROTOCOL_PORT = 0x01,
	CAN_PROTOCOL_PORT,
	ZIGBEE_PROTOCOL_PORT,
	RS485_PROTOCOL_PORT,
	I2C_PROTOCOL_PORT,
}PROTOCOL_TYPE;

/* 指令类型定义 */
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

typedef  struct
{
	uint8_t cmd;
	union
	{
		uint8_t reserve;
		uint8_t addr;
	}option;
	uint8_t data[128];  //串口发送每包bin数据内容需1024整除(len限制长度，建议128 Byte)
}data_info_t;


typedef struct
{
	uint8_t port;
	data_info_t data_info;
}protocol_info_t;

typedef struct
{
	uint8_t port;
	void (*handle)(uint8_t *data, uint8_t len);
}protocol_entry_t;

void JumpFirmwareSuccess(void);
void HandleUsartQueue(void);
void handle_can_queue(void);

#endif


