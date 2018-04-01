/*
 * basic_io.h
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */

#ifndef BASIC_IO_H_
#define BASIC_IO_H_

#define BIO_MAKE_BASE(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

#define LEDR_ADD 0x00000000
#define LEDR_BASE BIO_MAKE_BASE(LEDR_ADD)

#define SW_ADD 0x00001100
#define SW_BASE BIO_MAKE_BASE(SW_ADD)

void sw_read(void);
int8_t sw_get_bit_val(uint8_t bit);
int32_t sw_get_val(void);
void leds_set(uint8_t led, uint32_t value);
#endif /* BASIC_IO_H_ */
