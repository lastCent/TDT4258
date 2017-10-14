#include <stdint.h>
#include <stdbool.h>
#include "efm32gg.h"


void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	*TIMER1_IFC |= 0x1;
	*TIMER1_IEN = 0;	
	next = false;
}

/*
 * GPIO even pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	*GPIO_IFC = *GPIO_IF;
	uint32_t temp = ~*GPIO_PC_DIN;
	if ((temp & 0b1) == 0b1){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay2 = 1;
	}
	else if ((temp & 0b10) == 0b10){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
	}
	else if ((temp & 0b100) == 0b100){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
		}
	else if ((temp & 0b1000)== 0b1000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay2 = 1;
		}
	else if ((temp & 0b10000)== 0b10000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
		}
	else if ((temp & 0b100000)== 0b100000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
		}
	else if ((temp & 0b1000000)== 0b1000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
		}
	else if ((temp & 0b10000000)== 0b10000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
		}
}
/*
 * GPIO odd pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_EVEN_IRQHandler();
}

