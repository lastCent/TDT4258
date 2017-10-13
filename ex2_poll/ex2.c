#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "efm32gg.h"


/*
 * Declaration of peripheral setup functions 
 */
void setupTimer(uint32_t period);
void setupDAC();
void setupGPIO();
void playWave();
void playSound();
int duration = 1500;
uint32_t amp[16] = { 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3 };


// Set first number to be length of array, wave starts after 0th int
static int volume = 2;
static int cosine[16] = { 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3 };	// Cosine
static int saw[16] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 5, 3, 1 };	// Sawtooth 
static int sawInv[16] = { 1, 3, 5, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1 };	// Sawtooth inverted
static int square[16] = { 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 4, 4, 4 };	// SquaRE
static int sqrWigl[16] = { 1, 2, 1, 2, 4, 5, 4, 5, 1, 2, 1, 2, 4, 5, 4, 5 };	// Wiggly square
static int low[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	//none
//premade sound 1
static int *soundTune_1[7] = { sawInv, square, square, square, square, square, saw };
static int soundIntervals_1[7] = { 800, 800, 850, 800, 700, 900, 2000 };
static int soundDurations_1[7] = { 6000000, 3000000, 6000000, 6000000, 6000000, 6000000, 6000000 };
//premade sound 2
static int *soundTune_2[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
static int soundIntervals_2[7] = { 500, 1000, 500, 1000, 500, 1000, 500 };
static int soundDurations_2[7] = { 1000000, 2000000, 1000000, 2000000, 1000000, 2000000, 1000000 };

//premade sound 3
static int *soundTune_3[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
static int soundIntervals_3[7] = { 300, 400, 500, 600, 700, 800, 900 };
static int soundDurations_3[7] = { 1000000, 2000000, 3000000, 1000000, 2000000, 3000000, 1000000 };


int main(void)
{
	/*
	 * Call the peripheral setup functions 
	 */
	setupGPIO();
	setupDAC();
	setupTimer(SAMPLE_PERIOD);
	
	while (1) {
		//int *demoTune[10] = { sawInv, square, square, square, square, square, saw };
		//int demoIntervals[10] = { 800, 800, 850, 800, 700, 900, 2000 };
		//int demoDurations[10] = { 6000000, 3000000, 6000000, 6000000, 6000000, 6000000, 6000000 };
		// Play a tune
		playSound(demoTune, 10, 16, demoIntervals, demoDurations);
		// Debug end
		 *GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		 uint32_t temp = ~*GPIO_PC_DIN;
		 // Decide which button is pressed
		 if ((temp & 0b1) == 0b1){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b10) == 0b10){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b100) == 0b100){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b1000)== 0b1000){
		 playSound(soundTune_2, 7, 16, soundIntervals_2, soundDurations_2);
		 }else if ((temp & 0B10000)== 0b10000){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b100000)== 0b100000){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b1000000)== 0b1000000){
		 playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
		 }else if ((temp & 0b10000000)== 0b10000000){
		 playSound(soundTune_3, 7, 16, soundIntervals_3, soundDurations_3);
		 }
	}
	return 0;
}

void playSound(int **waveArr, int sizeWarr, int sizeWave, int *intervalArr, int *durationArr){
	for (int i = 0; i < sizeWarr; i++) {playWave(waveArr[i], sizeWave, *(intervalArr + i),
			*(durationArr + i));
	}
}

void playWave(int *wavePtr, int size, int interval, int duration){
	int playsLeft = duration / (interval * size);
	while (playsLeft > 0) {
		for (int i = 0; i < size; i++) {
			*TIMER1_CNT = 0;
			*DAC0_CH0DATA = wavePtr[i] * volume;
			*DAC0_CH1DATA = wavePtr[i] * volume;
			WHILE (*TIMER1_CNT < interval) ;
		}
		playsLeft--;
	}
}

