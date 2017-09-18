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
	.long   SysTick_Handler           /* SysTick Handler              */

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
	 .long   gpio_handler_2            /* GPIO odd handler */
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

/////////////////////////////////////////////////////////////////////////////
//
// Reset handler
// The CPU will start executing here after a reset
//
/////////////////////////////////////////////////////////////////////////////
.globl  SysTick_Handler
.type   SysTick_Handler, %function
.globl  _reset
.type   _reset, %function

.thumb_func
_reset: 

	//Enable GPIO
	LDR	R1,	=CMU_BASE
	LDR	R2,	[R1, 	CMU_HFPERCLKEN0]
	MOV	R3,	1
	LSL	R3,	R3,	CMU_HFPERCLKEN0_GPIO
	ORR	R2,	R2,	R3
	STR	R2,	[R1, 	CMU_HFPERCLKEN0]
 
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
	LSL 	R3, 	R3, 	R4
	STR	R3,	[R2]

	//setup buttons
	LDR 	R1,	=GPIO_PC_BASE
	LDR 	R2,	=GPIO_MODEL
	ADD	R3,	R1,	R2
	LDR 	R2, 	=GPIO_DOUT
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
	
	//Enable interrupt generation
	LDR     R0,     =GPIO_BASE
	LDR     R1,     =GPIO_IEN
	ADD     R2,     R1,     R0  
	MOV     R3,     0xff
	STR     R3,     [R2]

	//Setup systick	
	LDR R1, =SYSTICK_LOAD
	LDR R2, =0x500000
	STR R2, [R1]

	LDR R1, =SYSTICK_VAL
	STR R2, [R1]

	LDR R1, =SYSTICK_CTRL
	LDR R2, =0b110
	LDR R3, [R1]
	ORR R3, R2, R3
	STR R3, [R1]
		
	// Enable interrupt handling       
	LDR     R0,     =ISER0
	MOV	R8,	0x800
	MOV	R9,	0x2
	ADD	R8,	R8, 	R9    
	STR	R8,	[R0]

	LDR	R1,	=GPIO_BASE
	LDR	R2,	=GPIO_IF
	ADD	R3,	R1,	R2		//Loading the value of GPIO_IF    into R3
	LDR	R2,	=GPIO_IFC				
	ADD	R2,	R2,	R1		//Loads the address of GPIO_IFC into R2
	LDR 	R4, 	[R3]			//Clears the interrupt, writing GPIO_IF into GPIO_IFC
	STR	R4, 	[R2]
	
	LDR	R1,	=SCR			//Enable energy mode 2, deep sleep, and reenter deep sleep when returning from interrupts
	MOV	R0,	0b110
	LDR	R2,	[R1]
	ORR	R0,	R2,	R0
	STR	R0,	[R1]
		
	//Puts the GECKO into deep sleep
	wfi
	B .

.thumb_func
gpio_handler_2:
	//Loads the value of GPIO_PC_DIN, LSL 8 times
	LDR	R2,	=GPIO_PC_BASE
	LDR	R3,	=GPIO_DIN
	ADD	R4,	R2,	R3
	LDR	R4, 	[R4]
	LSL	R4,	R4,	8
	MOV	R0,	0xFFFFFFFF
	EOR	R4,	R0,	R4

	//EXORS the bit into GPIO_PA_DOUT
	LDR	R1,	=GPIO_PA_BASE
	LDR	R2,	=GPIO_DOUT
	ADD	R6,	R1,	R2
	LDR	R3, 	[R6]
	EOR	R7,	R4,	R3
	STR	R7,	[R6]

	LDR	R1,	=GPIO_BASE
	LDR	R2,	=GPIO_IF
	ADD	R3,	R1,	R2		//Loading the value of GPIO_IF    into R3
	LDR	R2,	=GPIO_IFC				
	ADD	R2,	R2,	R1		//Loads the address of GPIO_IFC into R2
	LDR 	R4, 	[R3]			//Clears the interrupt, writing GPIO_IF into GPIO_IFC
	STR	R4, 	[R2]
		
	BX LR

.thumb_func
gpio_handler:
	LDR	R1,	=GPIO_BASE
	LDR	R2,	=GPIO_IF
	ADD	R3,	R1,	R2		//Loading the value of GPIO_IF    into R3
	LDR	R2,	=GPIO_IFC				
	ADD	R2,	R2,	R1		//Loads the address of GPIO_IFC into R2
	LDR 	R4, 	[R3]			//Clears the interrupt, writing GPIO_IF into GPIO_IFC
	STR	R4, 	[R2]
	
	// CLKtoggle
	LDR R1, =SYSTICK_CTRL
	LDR R2, =0b1
	LDR R3, [R1]
	EOR R3, R2, R3
	STR R3, [R1]		
			
	LDR	R0,	=GPIO_PA_BASE
	LDR	R1,	=GPIO_DOUT
	ADD	R2,	R1,	R0

	MOV	R3,	0b1111111000000000
	STR	R3,	[R2]
	LDR	R8,	=SYSTICK_VAL
	LDR	R9,	=SYSTICK_LOAD
	LDR	R9,	[R9]
	step1:
		LDR	R0,	[R8]
		MOV	R1,	0xff0
		CMP	R0,	R1
		BGT	step1
		STR	R9,	[r8]
		B step2	
	step2:
		MOV	R3,	0b1111110100000000
		STR	R3,	[R2]
		LDR	R0,	[R8]
		MOV	R1,	0xff0
		CMP	R0,	R1
		BGT	step2
		STR	R9,	[r8]
		B step3	
	step3:
		MOV	R3,	0b1111101100000000
		STR	R3,	[R2]
		LDR	R0,	[R8]
		MOV	R1,	0xff0
		CMP	R0,	R1
		BGT	step3
		STR	R9,	[r8]
		B step4	
	step4:
		MOV	R3,	0b1111011100000000
		STR	R3,	[R2]
		LDR	R0,	[R8]
		MOV	R1,	0xff0
		CMP	R0,	R1
		BGT	step4
		STR	R9,	[r8]
		B step5		
	step5:
		MOV	R3,	0b1110111100000000
		STR	R3,	[R2]
		LDR	R0,	[R8]
		MOV	R1,	0xff0
		CMP	R0,	R1
		BGT	step5
		STR	R9,	[r8]
		B step6
	step6:
		//Turning of LEDS			
		MOV	R3,	0b0
		MVN	R3,	R3
		STR	R3,	[R2]
		// CLKtoggle
		LDR R1, =SYSTICK_CTRL
		LDR R2, =0b1
		LDR R3, [R1]
		EOR R3, R2, R3
		STR R3, [R1]		
		BX LR		

.thumb_func
SysTick_Handler:
//Turn pins on in pattern
	BX LR
		

.thumb_func
dummy_handler:  
	B .  // do nothing

NOP
