#ifndef __MAIN_H__
#define __MAIN_H__

#include "stm32f0xx.h"
#include "delay.h"
#include "crc16.h"
#include "led.h"
#include "iwdg.h"
#include "can.h"
#include "can_queue.h"
#include "bootloader.h"
#include "protocol_parse.h"


#define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
#define FLASH_SIZE                        (0x10000)  /* 64 kBytes */

#endif


