#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/*
 * function to setup the timer 
 */
void setupSCR()			//uint32_t period
{
	*SCR |= 0b100;

}
