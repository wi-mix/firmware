/*
 * seven_seg.c
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */
#include <hps.h>
#include <socal.h>
#include <stdint.h>

#include "seven_seg.h"

void display7Seg(uint32_t word){
    alt_write_word(HEX0_BASE, word);
    alt_write_word(HEX1_BASE, word>>4);
    alt_write_word(HEX2_BASE, word>>8);
    alt_write_word(HEX3_BASE, word>>12);
    alt_write_word(HEX4_BASE, word>>16);
    alt_write_word(HEX5_BASE, word>>20);
}

