#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

volatile int timeToPlay = 0;
volatile int timeToPlay2 = 0;
volatile int timeToPlay3 = 0;
int counter = 0;

void __attribute__ ((interrupt)) TIMER1_IRQHandler()
{

	*TIMER1_IFC |= 0x1;
	*TIMER1_IEN = 1;
	if (counter%10 == 0){
	*GPIO_PA_DOUT = (0b11111111 << 8);
	}else if (counter%13 == 1){
	*GPIO_PA_DOUT = (0b00000000 << 8);
	}
	counter++;
}

/*
 * GPIO even pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler()
{

	*TIMER1_IEN = 1;
	//Clear interrupt flags
	*GPIO_IFC |= *GPIO_IF;
	uint32_t temp = ~*GPIO_PC_DIN;
	//*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);

	// Decide which button is pressed
	if ((temp & 0b1) == 0b1){
		//*TIMER1_TOP = period[0];
		timeToPlay3 = 1;
	}
	else if ((temp & 0b10) == 0b10){
		timeToPlay3 = 1;
		


	}
	else if ((temp & 0b100) == 0b100){
		timeToPlay3 = 1;

		}
	else if ((temp & 0b1000)== 0b1000){
		timeToPlay = 1;

		}
	else if ((temp & 0b10000)== 0b10000){
		timeToPlay3 = 1;

		}
	else if ((temp & 0b100000)== 0b100000){
		timeToPlay3 = 1;

		}
	else if ((temp & 0b1000000)== 0b1000000){
		timeToPlay3 = 1;

		}
	else if ((temp & 0b10000000)== 0b10000000){
		timeToPlay2 = 1;
		}
	else{
		*GPIO_PA_DOUT=~0; // If something falls out of the desired clauses, light up all LEDs
	}

}

/*
 * GPIO odd pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_EVEN_IRQHandler();

}


