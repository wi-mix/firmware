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

int32_t* getADCChannel(int channel_num){
	int32_t* channel = 0;
	switch(channel_num) {
				case 0: channel = ADC_CH0_BASE; break;
				case 1: channel = ADC_CH1_BASE; break;
				case 2: channel = ADC_CH2_BASE; break;
				case 3: channel = ADC_CH3_BASE; break;
				case 4: channel = ADC_CH4_BASE; break;
				case 5: channel = ADC_CH5_BASE; break;
				case 6: channel = ADC_CH6_BASE; break;
				case 7: channel = ADC_CH7_BASE; break;
				default: channel = NULL; break;
	    	}
	return channel;
}
