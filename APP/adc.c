/*
 * adc.c
 *
 *  Created on: Mar 17, 2018
 *      Author: dannick
 */
#include <hps.h>
#include <socal.h>
#include <stdint.h>

#include "adc.h"

void ADCInit(void){
    // Auto Read From the ADC
    alt_write_word(ADC_AUTO_BASE, 1);
}
