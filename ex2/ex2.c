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
void playWave();
void playSound();
void play(int);
int timeToPlay = 0;
int timeToPlay2 = 0;
int timeToPlay3 = 0;
void setPeriod();
bool next = true;
/*
// Global sound pointers
int* waveP;
int* InterP;
int* DurP;
int size;
*/
// Define premade waves
// Set first number to be length of array, wave starts after 0th int
int volume = 1;
int cosine[16] = { 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3 };     // Cosine
int saw[16] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 5, 3, 1 };        // Sawtooth 
int sawInv[16] = { 1, 3, 5, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1 };     // Sawtooth inverted
int square[16] = { 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 4, 4, 4 };     // SquaRE
int sqrWigl[16] = { 1, 2, 1, 2, 4, 5, 4, 5, 1, 2, 1, 2, 4, 5, 4, 5 };// Wiggly square
int low[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };        // none
// Define different sounds
// premade sound 1
int *soundTune_1[7] = { sawInv, square, square, square, square, square, saw };
int soundIntervals_1[7] = { 800, 800, 850, 800, 700, 900, 2000 };
int soundDurations_1[7] = { 6000000, 3000000, 6000000, 6000000, 6000000, 6000000, 6000000 };// premade sound 2
int *soundTune_2[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
int soundIntervals_2[7] = { 500, 1000, 500, 1000, 500, 1000, 500 };
int soundDurations_2[7] = { 1000000, 2000000, 1000000, 2000000, 1000000, 2000000, 1000000 };

// premade sound 3
int *soundTune_3[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
int soundIntervals_3[7] = { 300, 400, 500, 600, 700, 800, 900 };
int soundDurations_3[7] = { 1000000, 2000000, 3000000, 1000000, 2000000, 3000000, 1000000 };

// premade sound 4
int *soundTune_4[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
int soundIntervals_4[7] = { 100, 100, 100, 100, 100, 100, 100 };
int soundDurations_4[7] = { 300, 300, 300, 300, 300, 300, 300 };

int interCount;
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
		if(timeToPlay){
			play(1);
			timeToPlay = 0;
		}
		else if(timeToPlay2){
			play(2);
			timeToPlay2 = 0;
		}
		else if (timeToPlay3){
			play(3);
			timeToPlay3 = 0;
		}
		*GPIO_PA_DOUT = (0b11111111 << 8);
		__asm__("wfi");
	}
	return 0;
}

void playSound(int **waveArr, int sizeWarr, int sizeWave, int *intervalArr, int *durationArr){
        // Loops through sound arrays and calls the method playWave()
        for (int i = 0; i < sizeWarr; i++) {
                playWave(waveArr[i], sizeWave, *(intervalArr + i), *(durationArr + i));
        }
}

void playWave(int *wavePtr, int size, int interval, int duration){
	// Play a single wave for a given duration
        int timeLeft = duration;
	//playsLeft: number of times the wave is payed to fill the duration
        while (timeLeft > 0) {
                for (int i = 0; i < size; i++) {
                        // Feed DAC to play sound, wavePtr is a predefined amplitude array, Volume is used as a multiplier
                    *DAC0_CH0DATA = wavePtr[i] * volume;
                    *DAC0_CH1DATA = wavePtr[i] * volume;
					setPeriod(interval);
					while(next){}
					next = true;
                        
                }
                timeLeft -= interval * size;   
        }
}

void play(int x){
	if(x == 1){
		playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
	}else if(x == 2){
		playSound(soundTune_2, 7, 16, soundIntervals_2, soundDurations_2);
	}else if (x == 3){
		playSound(soundTune_3, 7, 16, soundIntervals_3, soundDurations_3);
	}
}

void setupNVIC()
{
	*ISER0 = 1<<1; //0x802 old value; // Enable interrupt handling, gpio = 1 and 11, timer1  = 12, 0b110000000001
	*ISER0 |= 1<<11;
	*ISER0 |= 1<<12;
}

void setPeriod(uint32_t period){
	*TIMER1_TOP = period;
	*TIMER1_IEN = 1;
}

