#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void setupTimer()//uint32_t period
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_TIMER1;	// Enable timer
	*TIMER1_TOP = 0b1111111111;	// Set period
	*TIMER1_IEN = 0x0;	// Enable interrupt
	*TIMER1_CMD = 0x1;	// Start timer
}
