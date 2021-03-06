#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void setupGPIO()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO;	// enable GPIO clock 
	*GPIO_PA_CTRL = 2;	// set high drive strength
	*GPIO_PA_MODEH = 0x55555555;	// set pins A8-15 as output
	*GPIO_PA_DOUT = 0b1111111100000000;	// Turn off LEDs D4-D8 (LEDs are active low)


	*GPIO_PC_DOUT = 0xff; //Set pins to pull up
	*GPIO_PC_MODEL = 0x33333333; // set pins to input
	*GPIO_EXTIPSELL = 0x22222222; // select pins to trigger interrupts
	*GPIO_EXTIFALL = 0xff; // set interrupt to trigger on falling edge
	*GPIO_IEN = 0xff; // enable interrupts
}
