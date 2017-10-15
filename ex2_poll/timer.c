#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void setupTimer(uint16_t period) 
{
	
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;
	*TIMER1_TOP = 0b11111111111;	//set period
	*TIMER1_IEN = 0X1;	// Enable timer
	*ISER0 = 0b100000000000;	//Enable timer interrupt
	*TIMER1_CMD = 0x1;	//start timer
} 
