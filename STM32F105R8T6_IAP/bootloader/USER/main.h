#ifndef __MAIN_H__
#define __MAIN_H__

#include <string.h>
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "crc16.h"
#include "usart.h"  
#include "usart_queue.h"
#include "bootloader.h"
#include "protocol_parse.h"
 
#define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
#define FLASH_SIZE                        (0x10000)  /* 64 kBytes */

#endif

