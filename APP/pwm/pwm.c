/*
 * pwm.c
 *
 *  Created on: Mar 27, 2018
 *      Author: vanselow
 */
#include <hps.h>
#include <socal.h>
#include <stdint.h>
#include <stdio.h>
#include "../models/models.h"
#include "pwm.h"
#include "../tasks.h"

void  MotorTask(motor_command command) {
	void* motor = PWM1_BASE;
	printf("RoWooW we are now: %d on %d\r\n", command.state, command.motor_num);
	switch(command.motor_num){
		case 0: motor = PWM1_BASE; break;
		case 1: motor = PWM2_BASE; break;
		case 2: motor = PWM3_BASE; break;
		case 3: motor = PWM4_BASE; break;
		default: motor = PWM1_BASE; break;
	}

	alt_write_word(motor, MOTOR_SPEED * command.state);
}
