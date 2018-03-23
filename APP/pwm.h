/*
 * pwm.h
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */

#ifndef PWM_H_
#define PWM_H_

#define PWM_MAKE_BASE(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

// PWM Locations
#define PWM1_ADD 0x00000200
#define PWM1_BASE PWM_MAKE_BASE(PWM1_ADD)
#define PWM2_ADD 0x00000204
#define PWM2_BASE PWM_MAKE_BASE(PWM2_ADD)
#define PWM3_ADD 0x00000208
#define PWM3_BASE PWM_MAKE_BASE(PWM3_ADD)
#define PWM4_ADD 0x0000020C
#define PWM4_BASE PWM_MAKE_BASE(PWM4_ADD)

// PWM Constants
#define PWM_MAX 625000
#define PWM_INC   6250

#endif /* PWM_H_ */