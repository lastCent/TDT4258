        .syntax unified



	



	      .include "efm32gg.s"







	/////////////////////////////////////////////////////////////////////////////



	//



  // Exception vector table



  // This table contains addresses for all exception handlers



	//



	/////////////////////////////////////////////////////////////////////////////



	



        .section .vectors



	



	      .long   stack_top               /* Top of Stack                 */



	      .long   _reset                  /* Reset Handler                */



	      .long   dummy_handler           /* NMI Handler                  */



	      .long   dummy_handler           /* Hard Fault Handler           */



	      .long   dummy_handler           /* MPU Fault Handler            */



	      .long   dummy_handler           /* Bus Fault Handler            */



	      .long   dummy_handler           /* Usage Fault Handler          */



	      .long   dummy_handler           /* Reserved                     */



	      .long   dummy_handler           /* Reserved                     */



	      .long   dummy_handler           /* Reserved                     */



	      .long   dummy_handler           /* Reserved                     */



	      .long   dummy_handler           /* SVCall Handler               */



	      .long   dummy_handler           /* Debug Monitor Handler        */



	      .long   dummy_handler           /* Reserved                     */



	      .long   dummy_handler           /* PendSV Handler               */



	      .long   dummy_handler           /* SysTick Handler              */







	      /* External Interrupts */



	      .long   dummy_handler



	      .long   gpio_handler            /* GPIO even handler */



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   gpio_handler            /* GPIO odd handler */



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler



	      .long   dummy_handler







	      .section .text





CMU_BASE = 0X400C800

CMU_HFPERCLKEN0 = 0X044

CMU_HFPERCLKEN0_GPIO = 13



LDR R1, CMU_BASE_ADDR

LDR R2, [R1, #CMU_HFPERCLKEN0]

MOV R3, #1

LSL R3, R3, #CMU_HFPERCLKEN0_GPIO

ORR R2, R2, R3

STR R2, [R1, #CMU_HFPERCLKEN0]

LDR R4, =GPIO_PA_BASE
LDR R5, =GPIO_CTRL
MOV R6, #0X2
STR R6, [R4, R5]

LDR R4, =GPIO_PA_BASE
LDR R5, =GPIO_MODEH
MOV R6, #0X55555555
STR R6, [R4, R5]

LDR R4, =GPIO_PA_BASE
LDR R5, =GPIO_DOUT
MOV R6, #0b0100000000
STR R6, [R4, R5]





CMU_BASE_ADDR:

	.LONG CMU_BASE







	/////////////////////////////////////////////////////////////////////////////



	//



	// Reset handler



  // The CPU will start executing here after a reset



	//



	/////////////////////////////////////////////////////////////////////////////







	      .globl  _reset



	      .type   _reset, %function



        .thumb_func



_reset: 



	      b .  // do nothing



	



	/////////////////////////////////////////////////////////////////////////////



	//



  // GPIO handler



  // The CPU will jump here when there is a GPIO interrupt



	//



	/////////////////////////////////////////////////////////////////////////////



	



        .thumb_func



gpio_handler:  







	      b .  // do nothing



	



	/////////////////////////////////////////////////////////////////////////////



	



        .thumb_func



dummy_handler:  



        b .  // do nothing

