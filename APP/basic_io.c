/*
 * basic_io.c
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */
#include <hps.h>
#include <socal.h>
#include <stdint.h>

#include "basic_io.h"

#ifndef APP_BASIC_IO_C_
#define APP_BASIC_IO_C_

uint32_t sw_val = 0;

void sw_read(void){
	sw_val = 0x3FF & alt_read_word(SW_BASE);
}

int8_t sw_get_bit_val(uint8_t bit){
		return (sw_val >> bit) & 1;
}

int32_t sw_get_val(){
	return sw_val;
}

#endif /* APP_BASIC_IO_C_ */
