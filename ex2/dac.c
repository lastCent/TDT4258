#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void setupDAC()
{
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0;	// Enable DAC clock
	*DAC0_CTRL = 0x50010;	// Prescale DAC clock
	*DAC0_CH0CTRL = 0x1;	// Enable left audio channel
	*DAC0_CH1CTRL = 0x1;	// Enable right audio channel
}
