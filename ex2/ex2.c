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
void playMelody();
void playMelody2();
void playMelody3();
void setPeriod(uint32_t period);

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
		__asm__("wfi");

		if(timeToPlay == 1){
			playMelody();
		}
		else if(timeToPlay2 == 1){
			playMelody2();
		}
		else if (timeToPlay3 == 1){
			playMelody3();
		}
	}
	return 0;
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
	*ISER0 |= 1<<1; //0x802 old value; // Enable interrupt handling, gpio = 1 and 11, timer1  = 12, 0b110000000001
	*ISER0 |= 1<<11;
	*ISER0 |= 1<<12;
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
void playMelody(){

	*GPIO_PA_DOUT=0;
	timeToPlay=0;
	*GPIO_IEN = 0;

	int counter = 0;
	int maxCounter = 290000;
	setPeriod(period[4]);
	while (counter<maxCounter) counter+=1;
	setPeriod(period[1]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[4]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[1]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[4]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[1]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[4]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[1]);
	counter=0;
	while (counter<maxCounter) counter+=1;

	*GPIO_PA_DOUT=~0;
	*GPIO_IEN=0xff;
}
void playMelody2(){
	*GPIO_PA_DOUT=0;
	timeToPlay2=0;
	*GPIO_IEN = 0;

	int counter = 0;
	int maxCounter = 100000;
	setPeriod(period[0]);
	while (counter<maxCounter) counter+=1;
	setPeriod(period[7]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[0]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[7]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[0]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[7]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[0]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[7]);
	counter=0;
	while (counter<maxCounter) counter+=1;

	*GPIO_PA_DOUT=~0;
	*GPIO_IEN=0xff;
}

void playMelody3(){

	*GPIO_PA_DOUT=0;
	timeToPlay3=0;
	*GPIO_IEN = 0;
	
	int counter = 0;
	int maxCounter = 100000;
	setPeriod(period[0]);
	while (counter<maxCounter) counter+=1;
	counter=1000;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[1]);
	counter=5000;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[7]);
	counter=10000;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[5]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	setPeriod(period[3]);
	counter=0;
	while (counter<maxCounter) counter+=1;
	*GPIO_PA_DOUT=~0;
	*GPIO_IEN=0xff;
	
}
void setPeriod(uint32_t period){
	*TIMER1_TOP = period;
	*TIMER1_IEN = 1;
}