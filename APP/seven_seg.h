/*
 * seven_seg.h
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */

#ifndef SEVEN_SEG_H_
#define SEVEN_SEG_H_

#define HEX_MAKE_BASE(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

// Hex Locations
#define HEX0_ADD 0x00000100
#define HEX0_BASE HEX_MAKE_BASE(HEX0_ADD)
#define HEX1_ADD 0x00000110
#define HEX1_BASE HEX_MAKE_BASE(HEX1_ADD)
#define HEX2_ADD 0x00000120
#define HEX2_BASE HEX_MAKE_BASE(HEX2_ADD)
#define HEX3_ADD 0x00000130
#define HEX3_BASE HEX_MAKE_BASE(HEX3_ADD)
#define HEX4_ADD 0x00000140
#define HEX4_BASE HEX_MAKE_BASE(HEX4_ADD)
#define HEX5_ADD 0x00000150
#define HEX5_BASE HEX_MAKE_BASE(HEX5_ADD)

void display7Seg(uint32_t word);

#endif /* SEVEN_SEG_H_ */
