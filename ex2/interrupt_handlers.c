#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"


void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	*TIMER1_IFC |= 0x1;	// Clear flag
	*TIMER1_IEN = 0;	// Turn off timer interrupts
	next = false;	// next used in ex2.c as a "timer/clock"
}


void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	*GPIO_IFC = *GPIO_IF;	// Clear flag
	// check which button has been pressed
	uint32_t temp = ~*GPIO_PC_DIN;
	if ((temp & 0b1) == 0b1){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);	// flash the corresponding LED
		melody = 1;	// Variable from ex2.c to play correct sound/melody
	}else if ((temp & 0b10) == 0b10){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 4;
	}else if ((temp & 0b100) == 0b100){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 1;
	}else if ((temp & 0b1000)== 0b1000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 2;
	}else if ((temp & 0b10000)== 0b10000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 1;
	}else if ((temp & 0b100000)== 0b100000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 1;
	}else if ((temp & 0b1000000)== 0b1000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 1;
	}else if ((temp & 0b10000000)== 0b10000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		melody = 3;
	}
}

void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	// call EVEN handler
	GPIO_EVEN_IRQHandler();
}

