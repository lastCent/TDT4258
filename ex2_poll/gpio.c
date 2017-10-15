#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
    

void setupGPIO() 
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	// Enable GPIO clock
	*GPIO_PA_CTRL = 2;	// Set high drive strength
	*GPIO_PA_MODEH = 0x55555555;	// Set pins A8-15 as output
	*GPIO_PA_DOUT = 0x0700;	// Turn on LEDs D4-D8 (LEDs are active low)
	*GPIO_PC_DOUT = 0xff;	// Set pins to pull up
	*GPIO_PC_MODEL = 0x33333333;	// Set pins to input
} 
