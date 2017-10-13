#include <stdint.h>
#include <stdbool.h>
#include "efm32gg.h"

volatile int timeToPlay;
volatile int timeToPlay2;
volatile int timeToPlay3;

void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{
	*TIMER1_IFC |= 0x1;
	*TIMER1_IEN = 0;	
}

/*
 * GPIO even pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{
	timeToPlay = 1;
	//*TIMER1_IEN = 1;
	//Clear interrupt flags
	*GPIO_IFC |= *GPIO_IF;
//	*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
	uint32_t temp = *GPIO_PC_DIN;
	//*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);

	// Decide which button is pressed
	if ((temp & 0b1) == 0b1){
		//*TIMER1_top = period[0];
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
	}
	else if ((temp & 0b10) == 0b10){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
	}
	else if ((temp & 0b100) == 0b100){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
		}
	else if ((temp & 0b1000)== 0b1000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay = 1;
		}
	else if ((temp & 0b10000)== 0b10000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
		}
	else if ((temp & 0b100000)== 0b100000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
		}
	else if ((temp & 0b1000000)== 0b1000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay3 = 1;
		}
	else if ((temp & 0b10000000)== 0b10000000){
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		timeToPlay2 = 1;
		}
}

/*
 * GPIO odd pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_EVEN_IRQHandler();

}


