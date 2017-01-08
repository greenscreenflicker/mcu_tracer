/*
 * leds.c
 *
 *  Created on: Jun 20, 2016
 *      Author: mmh
 */


#include "MK22F51212.h"

void init_leds(void){
	//PORTA1
	SIM_SCGC5|=SIM_SCGC5_PORTA_MASK;
	GPIOA_PDDR|=(1<<1);
	PORTA_PCR1 = PORT_PCR_MUX(1)|PORT_PCR_DSE_MASK;


}

void toogle_red(void){
	GPIOA_PTOR=(1<<1);
}


void red_on(void){
	GPIOA_PSOR=(1<<1);
}

void red_off(void){
	GPIOA_PCOR=(1<<1);
}

