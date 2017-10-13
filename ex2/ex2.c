#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
/*
 * TODO calculate the appropriate sample period for the sound wave(s) you 
 * want to generate. The core clock (which the timer clock is derived
 * from) runs at 14 MHz by default. Also remember that the timer counter
 * registers are 16 bits. 
 */
/*
 * The period between sound samples, in clock cycles 
 */
//#define   SAMPLE_PERIOD   0xf

/*
 * Declaration of peripheral setup functions 
 */
void setupTimer(); //uint32_t period
void setupDAC();
void setupNVIC();
void setupGPIO();
void setupSCR();
void setPeriod(uint32_t period);
void playWave();
void playSound();
volatile int timeToPlay;
volatile int timeToPlay2;
volatile int timeToPlay3;

/*
// Define premade waves
// Set first number to be length of array, wave starts after 0th int
static int volume = 1;
static int cosine[16] = { 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3 };	// Cosine
static int saw[16] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 5, 3, 1 };	// Sawtooth 
static int sawInv[16] = { 1, 3, 5, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1 };	// Sawtooth inverted
static int square[16] = { 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 4, 4, 4 };	// SquaRE
static int sqrWigl[16] = { 1, 2, 1, 2, 4, 5, 4, 5, 1, 2, 1, 2, 4, 5, 4, 5 };// Wiggly square
static int low[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	// none
// Define different sounds
// premade sound 1
static int *soundTune_1[7] = { sawInv, square, square, square, square, square, saw };
static int soundIntervals_1[7] = { 800, 800, 850, 800, 700, 900, 2000 };
static int soundDurations_1[7] = { 6000000, 3000000, 6000000, 6000000, 6000000, 6000000, 6000000 };

// premade sound 2
static int *soundTune_2[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
static int soundIntervals_2[7] = { 500, 1000, 500, 1000, 500, 1000, 500 };
static int soundDurations_2[7] = { 1000000, 2000000, 1000000, 2000000, 1000000, 2000000, 1000000 };

// premade sound 3
static int *soundTune_3[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
static int soundIntervals_3[7] = { 300, 400, 500, 600, 700, 800, 900 };
static int soundDurations_3[7] = { 1000000, 2000000, 3000000, 1000000, 2000000, 3000000, 1000000 };

// premade sound 4
static int *soundTune_4[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
static int soundIntervals_4[7] = { 100, 100, 100, 100, 100, 100, 100 };
static int soundDurations_4[7] = { 300, 300, 300, 300, 300, 300, 300 };
*/
/*
 * Your code will start executing here 
 */
int main(void)
{
	/*
	 * Call the peripheral setup functions 
	 */
	setupGPIO();
	setupDAC();
	setupTimer();
	setupSCR();
	/*
	 * Enable interrupt handling 
	 */
	setupNVIC();

	/*
	 * TODO for higher energy efficiency, sleep while waiting for
	 * interrupts instead of infinite loop for busy-waiting 
	 */
	//wfi;
	//extern timeToPlay=1;
	//timeToPlay=1;
	while (1){
		setPeriod(1000);

		if(timeToPlay == 1){
			//playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		}
		else if(timeToPlay2 == 1){
			//playSound(soundTune_2, 7, 16, soundIntervals_2, soundDurations_2)
		}
		else if (timeToPlay3 == 1){
			//playSound(soundTune_3, 7, 16, soundIntervals_3, soundDurations_3);
		}
		timeToPlay = 0;
		timeToPlay2 = 0;
		timeToPlay3 = 0;
		//__asm__("wfi");
	}
	return 0;
}
/*
void playSound(int **waveArr, int sizeWarr, int sizeWave, int *intervalArr, int *durationArr){
	// Loops through sound arrays and calls the method playWave()
	for (int i = 0; i < sizeWarr; i++) {
		playWave(waveArre[i], sizeWave, *(intervalArr + i), *(durationArr + i));
	}
}

void playWave(int *wavePtr, int size, int interval, int duration){
	// Play a single wave for a given duration
	int playsLeft = duration / (interval * size);
	// playsLeft: number of times the wave is played to fill the duration
	while (playsLeft > 0) {
		for (int i = 0; i < size; i++) {
			*TIMER1_CNT = 0;	// resest timer
			// Feed DAC to play sound, wavePTR is a predefined amplitude array, Volume is used as a multiplier
			*DAC0_CH0DATA = wavePtr[i] * volume;
			*DAC0_CH1DATA = wavePtr[i] * volume;
			//WAIT FUNCTION ie TIMER1_CNT
		}
		playsLeft--;
	}
}
*/
void setupNVIC()
{
	/*
	 * TODO use the NVIC ISERx registers to enable handling of
	 * interrupt(s) remember two things are necessary for interrupt
	 * handling: - the peripheral must generate an interrupt signal - the
	 * NVIC must be configured to make the CPU handle the signal You will
	 * need TIMER1, GPIO odd and GPIO even interrupt handling for this
	 * assignment. 
	 */
	*ISER0 |= 1<<1; //0x802 old value; // Enable interrupt handling, gpio = 1 and 11, timer1  = 12, 0b110000000001
	*ISER0 |= 1<<11;
	*ISER0 |= 1<<12;
}

void setPeriod(uint32_t period){
	*TIMER1_TOP = period;
	*TIMER1_IEN = 1;
}
