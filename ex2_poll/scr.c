#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
    

void setupSCR()			//uint32_t period
{
	
*SCR |= 0b100;

 
} 
