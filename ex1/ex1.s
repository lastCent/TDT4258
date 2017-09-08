
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


	      .section .text

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
cmu_base_addr:
	.long CMU_BASE

	//Enable GPIO
	CMU_BASE = 0x400c8000
	CMU_HFPERCLKEN0 = 0x044
	CMU_HFPERCLKEN0_GPIO = 13
	LDR	R1,	cmu_base_addr
	LDR	R2,	[R1, #CMU_HFPERCLKEN0]
	MOV	R3,	#1
	LSL	R3,	R3,	#CMU_HFPERCLKEN0_GPIO
	ORR	R2,	R2,	R3
	STR	R2,	[R1, #CMU_HFPERCLKEN0]
 
	// Write 0x2 to GPIO_PA_CTRL
	LDR	R0,	=GPIO_PA_BASE
	LDR	R1,	=GPIO_CTRL
	ADD	R2,	R1,	R0
	MOV	R3,	0x2
	STR	R3,	[R2]
	// Set pins 8-15 to output
	LDR	R0,	=GPIO_PA_BASE
	LDR 	R1,	=GPIO_MODEH
	ADD	R2,	R1,	R0
	MOV	R3,	0x55555555
	STR	R3,	[R2]
	//Turn pins off
	LDR	R0,	=GPIO_PA_BASE
	LDR	R1,	=GPIO_DOUT
	ADD	R2,	R1,	R0
	MOV 	R4,  	8
	MOV	R3,	0b11111111
	LSL 	R3, R3, R4
	STR	R3,	[R2]

	//setup buttons
	LDR 	R1, =GPIO_PC_BASE
	LDR 	R2, =GPIO_MODEL
	ADD	R3,	R1,	R2
	LDR 	R2, =GPIO_DOUT
	ADD	R4,	R1,	R2	
	MOV	R1, 	0x33333333
	MOV	R2,	0xff
	STR	R1,	[R3]	//Setting Pins to input
	STR	R2,	[R4]	//Setting pins to pull up mode

	//Enable GPIO interrupts
        //Set GPIO_EXTIPSELL
        LDR     R0,     =GPIO_BASE
        LDR     R1,     =GPIO_EXTIPSELL
        ADD     R2,     R1,     R0  
        MOV     R3,     0x22222222
        STR     R3,     [R2]
        //Enable interrupt 1->0
        LDR     R0,     =GPIO_BASE
        LDR     R1,     =GPIO_EXTIFALL
        ADD     R2,     R1,     R0  
        MOV     R3,     0xff
        STR     R3,     [R2]
        //Enable interrupt 0->1
        LDR     R0,     =GPIO_BASE
        LDR     R1,     =GPIO_EXTIRISE
        ADD     R2,     R1,     R0  
        MOV     R3,     0xff
        STR     R3,     [R2]
        //Emable interrupt generation
        LDR     R0,     =GPIO_BASE
        LDR     R1,     =GPIO_IEN
        ADD     R2,     R1,     R0  
        MOV     R3,     0xff
        STR     R3,     [R2]
        // Enable interrupt handling
        LDR     R0,     =ISER0
yay:
	.long homo
	homo = 0x802
        MOV     R1,     yay
	STR	R1,	[R0]    


	

loop: 
	LDR	R0,	=GPIO_PA_BASE
	LDR	R1,	=GPIO_DOUT
	ADD	R2,	R1,	R0
	MOV 	R4,  	8
	MOV	R3,	0b11111111
	LSL 	R3, R3, R4
	STR	R3,	[R2]
	b loop

	/////////////////////////////////////////////////////////////////////////////
	//
  // GPIO handler
  // The CPU will jump here when there is a GPIO interrupt
	//
	/////////////////////////////////////////////////////////////////////////////
	
        .thumb_func
gpio_handler:
	

	LDR	R0,	=GPIO_PA_BASE
	LDR	R1,	=GPIO_DOUT
	ADD	R2,	R1,	R0
	MOV 	R4,  	8
	MOV	R3,	0b00000000
	LSL 	R3, R3, R4
	STR	R3,	[R2]
	
	
	LDR	R1,	=GPIO_BASE
	LDR	R2,	=GPIO_IF
	ADD	R3,	R1,	R2				//Loading the value of GPIO_IF    into R3
	LDR	R2,	=GPIO_IFC				
	ADD	R2,	R2,	R1				//Loads the address of GPIO_IFC into R2
	LDR 	R4, 	[R3]
	STR	R4, 	[R2]					//Clears the interrupt, writing GPIO_IF into GPIO_IFC
	
	/*
	//Loads the value of GPIO_PC_DIN, LSL 8 times
	LDR	R0,	=GPIO_PC_BASE
	LDR	R1,	=GPIO_DIN
	ADD	R2,	R1,	R0
	LSL	R2,	R2,	8
	LDR	R0,	=GPIO_PC_BASE
	LDR	R1,	=GPIO_DOUT
	ADD 	R3, 	R1,	R0
	STR	R2, 	[R3]
	*/

	b loop
	/////////////////////////////////////////////////////////////////////////////

        .thumb_func
dummy_handler:  
        b .  // do nothing


