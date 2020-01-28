/**
  ******************************************************************************
  * @file           : delayus_lib.c
  * @brief          : Implements Functions to delay in microseconds
  ******************************************************************************
  *
  * @author			: Felix Lohse & Arne Bruhns
  *
  *
  ******************************************************************************
  */

#include "delayus_lib.h"

/**
  * @brief Function to create a delay in microseconds
  * @param us to wait
  * @retval None
  * @pragmas used to turn off compiler optimization to ensure delay time is actually close to 1 us
  */

#pragma GCC push_options
#pragma GCC optimize ("O0")
inline void delayUs(uint16_t us) {
	volatile uint16_t counter = 3 * us;	// 3 iterations give 0,06us difference per 1 us
	while(counter--);
}
#pragma GCC pop_options


/*
void delayUs(uint16_t us){
	do {
	asm volatile (

			"MOV R0,%[loops]\n\t"\
			"1: \n\t"\
			"SUB R0, #1\n\t"\
			"CMP R0, #0\n\t"\
			"BNE 1b \n\t" : : [loops] "r" (6*us) : "memory"\

		      );
	} while(0);
}

*/
