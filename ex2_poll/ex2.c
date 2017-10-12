#include <stdint.h>
#include <stdlib.h>
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
#define   SAMPLE_PERIOD   0

/*
 * Declaration of peripheral setup functions 
 */
void setupTimer(uint32_t period);
void setupDAC();
void setupNVIC();
void setupGPIO();
void melody();
void melody1();
void melody2();
void melody3();
void playWave();
void playSound();
int duration = 1500;
uint32_t amp[16] = {4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3};
uint32_t counter =0;
uint32_t ampIndex = 0;
uint32_t period[5] = {50, 200, 500, 800, 1000};
int p1[5] = {4, 3, 2, 1, 0};
int p2[6] = {1, 4, 1, 4, 1, 4};
int p3[5] = {0, 1, 2, 3, 4};

// Set first number to be length of array, wave starts after 0th int
int sound1[17] = {17,4,5,6,7,7,7,6,5,4,3,2,1,1,1,2,3}; // Cosine
int saw[17] = {17,1,1,2,2,3,3,4,4,5,5,6,6,7,5,3,1}; // Sawtooth 
int sawInv[17] = {17,1,3,5,7,7,6,6,5,5,4,4,3,3,2,2,1}; // Sawtooth inverted
int square[17] = {17,1,1,1,1,4,4,4,4,1,1,1,1,4,4,4,4}; // Square
int sqrWigl[17] = {17,1,2,1,2,4,5,4,5,1,2,1,2,4,5,4,5}; // Wiggly square
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
	setupTimer(SAMPLE_PERIOD);

	/*
	 * Enable interrupt handling 
	 */
	//setupNVIC();

	/*	
	 * TODO for higher energy efficiency, sleep while waiting for
	 * interrupts instead of infinite loop for busy-waiting 
	 */
	while (1){
		// Create pointers to waves
		int *s;
		s = sawInv;
		int *t;
		t = saw;
		int *u;
		u = square;
		int *v;
		v = sqrWigl;
		//Create pointers to input arrays
		int* demoTune[10] = {s,t,s,t,s,t,u,u,u,v};
		int demoIntervals[11] = {11,1000,1000,500,500,300,300,1000,500,300,800};
		int demoDurations[10] = {90000,90000,90000,90000,90000,90000,90000,90000,90000,90000};
		int** demoT;
		demoT = demoTune;
		int* demoI;
		demoI = demoIntervals;
		int* demoD;
		demoD = demoDurations;
		// Play a tune
		playSound(**demoT, *demoI, *demoD);
		//playWave(*s, 1000, 900000);
		//playWave(*t, 1000, 900000);
		//playWave(*s, 500, 900000);
		//playWave(*t, 500, 900000);
		//playWave(*s, 300, 900000);
		//playWave(*t, 300, 900000); 
		//playWave(*u, 1000, 900000);
		//playWave(*u, 500, 900000);
		//playWave(*u, 300, 900000);
		//playWave(*v, 800, 900000);
		// Debug end
		/*
		*GPIO_PA_DOUT = (*GPIO_PC_DIN << 8);
		uint32_t temp = ~*GPIO_PC_DIN;
		// Decide which button is pressed
		if ((temp & 0b1) == 0b1){
			melody(1);
		}else if ((temp & 0b10) == 0b10){
			melody(1);
		}else if ((temp & 0b100) == 0b100){
			melody(1);
		}else if ((temp & 0b1000)== 0b1000){
			melody(0);
		}else if ((temp & 0b10000)== 0b10000){
			melody(1);
		}else if ((temp & 0b100000)== 0b100000){
			melody(1);
		}else if ((temp & 0b1000000)== 0b1000000){
			melody(1);
		}else if ((temp & 0b10000000)== 0b10000000){
			melody(2);
		}
		*/
	}
	return 0;
}

void melody(int x){
	int *p;
	if(x == 0){
		p = p1;
	}else if(x == 1){
		p = p2;
	}else{
		p = p3;
	}

	for(int i = 0; i < sizeof(p) + 1; i++){
		while(counter<duration){
			if(*TIMER1_CNT > period[p[i]]){
				*TIMER1_CNT = 0;
				*DAC0_CH0DATA = amp[ampIndex];
				*DAC0_CH1DATA = amp[ampIndex];
				ampIndex++;
				if (ampIndex == 16){ ampIndex=0;}
				counter++;			
			}
		}
		counter = 0;
	}
}

void playWave(int* wavePtr, int interval, int duration) {
	int playsLeft = duration / (interval * (sizeof(sound1)/sizeof(int)));
	while(playsLeft > 0) {
		for(int i = 1; i < *wavePtr/sizeof(int); i++){
			*TIMER1_CNT = 0;
			*DAC0_CH0DATA = *(wavePtr + i);
			*DAC0_CH1DATA = *(wavePtr + i);
			while(*TIMER1_CNT < interval);
		}
		playsLeft --;
	}
}

void playSound(int** waveArr, int* intervalArr, int* durationArr) {
	for (int i = 1; i < **waveArr/sizeof(int); i++) {
		playWave(waveArr[i - 1], *(intervalArr + i), *(durationArr -1 + i));
	} 
}

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
}

/*
 * if other interrupt handlers are needed, use the following names:
 * NMI_Handler HardFault_Handler MemManage_Handler BusFault_Handler
 * UsageFault_Handler Reserved7_Handler Reserved8_Handler
 * Reserved9_Handler Reserved10_Handler SVC_Handler DebugMon_Handler
 * Reserved13_Handler PendSV_Handler SysTick_Handler DMA_IRQHandler
 * GPIO_EVEN_IRQHandler TIMER0_IRQHandler USART0_RX_IRQHandler
 * USART0_TX_IRQHandler USB_IRQHandler ACMP0_IRQHandler ADC0_IRQHandler
 * DAC0_IRQHandler I2C0_IRQHandler I2C1_IRQHandler GPIO_ODD_IRQHandler
 * TIMER1_IRQHandler TIMER2_IRQHandler TIMER3_IRQHandler
 * USART1_RX_IRQHandler USART1_TX_IRQHandler LESENSE_IRQHandler
 * USART2_RX_IRQHandler USART2_TX_IRQHandler UART0_RX_IRQHandler
 * UART0_TX_IRQHandler UART1_RX_IRQHandler UART1_TX_IRQHandler
 * LEUART0_IRQHandler LEUART1_IRQHandler LETIMER0_IRQHandler
 * PCNT0_IRQHandler PCNT1_IRQHandler PCNT2_IRQHandler RTC_IRQHandler
 * BURTC_IRQHandler CMU_IRQHandler VCMP_IRQHandler LCD_IRQHandler
 * MSC_IRQHandler AES_IRQHandler EBI_IRQHandler EMU_IRQHandler 
 */
