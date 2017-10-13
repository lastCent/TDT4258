#include <stdint.h>
#include <stdbool.h>
#include "efm32gg.h"

volatile int timeToPlay = 0;
volatile int timeToPlay2 = 0;
volatile int timeToPlay3 = 0;
volatile int isOff = 0;
int duration = 1500;
uint32_t amp[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};
uint32_t counter =0;
uint32_t period[8] = {0b1111111111,0b1111110010,0b1111100000,0b1110000000, 0b1000000000, 0b1000011011, 0b1000111111, 0b1011111111  };
/*
// Global sound pointers
int* waveP;
int* InterP;
int* DurP;
int size;

// Define premade waves
// Set first number to be length of array, wave starts after 0th int
int interCounter;
int volume;
int cosine[16];    // Cosine
int saw[16];    // Sawtooth 
int sawInv[16]; // Sawtooth inverted
int square[16];// SquaRE
int sqrWigl[16];// Wiggly square
int low[16];        // none
// Define different sounds
// premade sound 1
int *soundTune_1[7];
int soundIntervals_1[7];
int soundDurations_1[7];
// premade sound 2
int *soundTune_2[7];
int soundIntervals_2[7];
int soundDurations_2[7];

// premade sound 3
int *soundTune_3[7];
int soundIntervals_3[7];
int soundDurations_3[7];

// premade sound 4
int *soundTune_4[7];
int soundIntervals_4[7];
int soundDurations_4[7];
*/

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

	//*TIMER1_IEN = 1;
	//Clear interrupt flags
	*GPIO_IFC |= *GPIO_IF;
	uint32_t temp = ~*GPIO_PC_DIN;
	*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);

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
	/*
	if(isOff == 1){ // check if timer is enabled or not
		isOff = 0;
		*TIMER1_IEN = 1;
	}

	counter=0; // reset the tone time
	*/



	//*GPIO_PA_DOUT ^= ~(*GPIO_PC_DIN << 8);
	//*GPIO_PA_DOUT = ~(tone << 8);
}

/*
 * GPIO odd pin interrupt handler 
 */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler()
{
	GPIO_EVEN_IRQHandler();

}


