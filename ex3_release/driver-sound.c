#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h> 		// Memory IO reservation
#include <linux/interrupt.h>  	// Used for interrupt tasklets
#include <linux/signal.h>  		// Signal usage
#include <linux/sched.h>  		// Used for current PID
#include <linux/fs.h>  			// Get device number
#include <linux/cdev.h>  		// For char dev registration
#include <linux/device.h> 		// For device creation (c1)
#include <asm/uaccess.h>  		// Userspace read/write


//Declare memory constants
#define DAC0_BASE2 0x40004000

#define DAC0_CTRL     ((volatile uint32_t*)(DAC0_BASE2 + 0x000))
#define DAC0_CH0CTRL  ((volatile uint32_t*)(DAC0_BASE2 + 0x008))
#define DAC0_CH1CTRL  ((volatile uint32_t*)(DAC0_BASE2 + 0x00c))
#define DAC0_IEN      ((volatile uint32_t*)(DAC0_BASE2 + 0x010))
#define DAC0_IF       ((volatile uint32_t*)(DAC0_BASE2 + 0x014))
#define DAC0_IFS      ((volatile uint32_t*)(DAC0_BASE2 + 0x018))
#define DAC0_IFC      ((volatile uint32_t*)(DAC0_BASE2 + 0x01c))
#define DAC0_CH0DATA  ((volatile uint32_t*)(DAC0_BASE2 + 0x020))
#define DAC0_CH1DATA  ((volatile uint32_t*)(DAC0_BASE2 + 0x024))

#define TIMER3_BASE 0x40010c00

#define TIMER3_CMD ((volatile uint32_t*)(TIMER3_BASE + 0x04))
#define TIMER3_IEN ((volatile uint32_t*)(TIMER3_BASE + 0x0c))
#define TIMER3_IFC ((volatile uint32_t*)(TIMER3_BASE + 0x18))
#define TIMER3_TOP ((volatile uint32_t*)(TIMER3_BASE + 0x1c))
#define TIMER3_CNT ((volatile uint32_t*)(TIMER3_BASE + 0x24))
#define TIMER3_IF  ((volatile uint32_t*)(TIMER3_BASE + 0x10)


// Define premade waves
// Set first number to be length of array, wave starts after 0th int
int volume = 1;	// variable used as a multiplier for amplitude of waves
int cosine[16] = { 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 1, 1, 2, 3 };    // Cosine
int saw[16] = { 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 5, 3, 1 };       // Sawtooth 
int sawInv[16] = { 1, 3, 5, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1 };    // Sawtooth inverted
int square[16] = { 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 4, 4, 4, 4 };    // SquaRE
int sqrWigl[16] = { 1, 2, 1, 2, 4, 5, 4, 5, 1, 2, 1, 2, 4, 5, 4, 5 };	// Wiggly square
int low[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };       // none
int low4[4] = {0, 0, 0, 0};
int hSpike[4] = {8, 7, 5, 1}; 						// retro sound

// Define different sounds
//------------------------
// premade sound 1
int *soundTune_1[7] = { sqrWigl, sqrWigl, sqrWigl, sqrWigl, sqrWigl, sqrWigl, sqrWigl };
int soundIntervals_1[7] = { 24000, 24000, 24000, 14000, 14000, 14000, 14000 };
int soundDurations_1[7] = { 3000000, 3000000, 3000000, 3000000, 3000000, 3000000, 3000000 };

// premade sound 2
int *soundTune_2[7] = { cosine, sqrWigl, cosine, sqrWigl, cosine, sqrWigl, cosine};
int soundIntervals_2[7] = { 10000, 1000, 5000, 1000, 5000, 1000, 5000 };
int soundDurations_2[7] = { 1000000, 2000000, 1000000, 2000000, 1000000, 2000000, 1000000 };

// premade sound 3
int *soundTune_3[7] = { low, sqrWigl, low, sqrWigl, cosine, low, low};
int soundIntervals_3[7] = { 3000, 4000, 5000, 6000, 7000, 8000, 9000 };
int soundDurations_3[7] = { 1000000, 2000000, 3000000, 1000000, 2000000, 3000000, 1000000 };

// Start up melody, split into separate parts for the sake of formatting and easy configuration
//---------------------------------------------------------------------------------------------
// short burst of sound
int *soundTune_4[6] = { low4, hSpike, low4, hSpike, low4, low4};
int soundIntervals_4[6] = { 6000, 6000,  6000, 6000, 6000, 6000};
int soundDurations_4[6] = { 3500000, 2000000, 2500000, 4000000, 3000000, 4000000};

// short burst, bounce sound
int *soundTune_5[6] = {low4, hSpike, low4, hSpike, low4, hSpike};
int soundIntervals_5[6] = {12000, 12000,  12000, 12000, 12000, 20000};
int soundDurations_5[6] = {500000, 500000, 500000, 500000, 500000, 500000};

// soundTune_6
int *soundTune_6[4] = {cosine, cosine, cosine, cosine};
int soundIntervals_6[4] = {10000, 10000, 10000, 10000};
int soundDurations_6[4] = {3000000, 3000000, 3000000, 3000000};

// soundTune_6_5
int *soundTune_6_5[4] = {cosine, cosine, cosine, cosine};
int soundIntervals_6_5[4] = {20000, 20000, 20000, 20000};
int soundDurations_6_5[4] = {3000000, 3000000, 3000000, 3000000};

// melody part3
int *soundTune_7[8] = {hSpike, low, hSpike, low, hSpike, low, hSpike, low};
int soundIntervals_7[8] = {3000, 3000, 8000, 8000, 3250, 3250, 8000, 8000};
int soundDurations_7[8] = {3500000, 1000000, 3500000, 1000000, 3500000, 1000000, 3500000, 1000000};

//melody part3.5
int *soundTune_8[8] = {hSpike, low, hSpike, low, hSpike, low, hSpike, low};
int soundIntervals_8[8] = {3500, 3500, 8000, 8000, 5000, 5000, 8000, 8000};
int soundDurations_8[8] = {3500000, 1000000, 3500000, 1000000, 3500000, 1000000, 3500000, 1000000};



/*
 * Declare function prototypes
 */
static int SOUND_open (struct inode *, struct file *);
static int SOUND_release(struct inode *, struct file *);
static ssize_t SOUND_read(struct file *, char *, size_t, loff_t *);
static ssize_t SOUND_write(struct file *, const char *, size_t, loff_t *);
static irqreturn_t KERNEL_IRQ_HANDLER(int, void*);

void setPeriod(uint32_t);
void play(size_t);
void playWave(int *, int, int, int);
void playSound(int **, int , int, int *, int *);

/*
 * Global variables: 
 */
volatile bool next;
const char device_name[] = "SOUND";	// Name of driver
const char device_class[] = "SOUND_C";
dev_t *dev_num; 					// Device ID, NULL if shared
static int majorNumber;
struct task_struct *task;  			// Takes pid number of child process
static struct class* c1;  			// Device class
static struct device* d1;  			// Device file

// Supported file operations struct
const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = SOUND_open,
	.release = SOUND_release,
	.read = SOUND_read,
	.write = SOUND_write,
};



static int __init SOUND_init(void)
{
	int err = 0;  				// Error checking variable
	struct cdev *sound_cdev;	// Kernel char device structure
	// Allocate major number (id for device)
	majorNumber = register_chrdev(0, device_name, &fops);
	*dev_num = MKDEV(majorNumber, 0);
	printk("driver sound device_num %d", majorNumber);
	if (majorNumber<0) goto return_err;
	
	// Register char device
	sound_cdev = cdev_alloc();
	cdev_init(sound_cdev, &fops);
	err = cdev_add(sound_cdev, MKDEV(majorNumber, 0), 1);
	if (err) goto unregister_dev;
	
	// Make visible driver file in /dev/
	c1 = class_create(THIS_MODULE, device_class);
	if (IS_ERR(c1)) {
		err = -1;
		goto unregister_dev;
	}
	d1 = device_create(c1, NULL, MKDEV(majorNumber, 0), NULL, device_name);
	if (IS_ERR(d1)) {
		err = -1;
		goto destroy_class;
	}

	return 0;
	
	// Cleanup functions:
	destroy_class: class_destroy(c1);
	unregister_dev: unregister_chrdev_region(MKDEV(majorNumber, 0), 1);
	return_err: printk("failing"); return err;
}

static void __exit SOUND_cleanup(void)
{
	*DAC0_CH0CTRL = 0x0;	// Disable left audio channel
	*DAC0_CH1CTRL = 0x0;	// Disable right audio channel edge
	
	*TIMER3_IEN = 0x0;	// disable interrupt
	*TIMER3_CMD = 0x2;	// STOP timer
	// Remove device file
	device_destroy(c1, MKDEV(majorNumber, 0));
	// Remove class
	class_destroy(c1);
	// Free device (number)
	unregister_chrdev_region(MKDEV(majorNumber, 0), 1);
}

static int SOUND_open (struct inode *inode, struct file *filp) 
{
	int err;  // Error checking variable
	void* region;  // request_mem_region error checking
	
	// Register client process
	task = current;
	
	// Allocate IO regions
	region = request_mem_region(TIMER3_BASE, 0x24, device_name);
	printk("DEBUG: Sound driver opened! :D \n");
	if (region == NULL) {
		err = -1;
		goto return_err;
	}
	region = request_mem_region((int) DAC0_BASE2, 0x24, device_name); 
	if (region ==NULL) {
		err = -1;
		goto release_reg1;
	} 
	// Perform DAC setup
	*DAC0_CTRL = 0x50010;	// Prescale DAC clock
	*DAC0_CH0CTRL = 0x1;	// Enable left audio channel
	*DAC0_CH1CTRL = 0x1;	// Enable right audio channel edge
	
		// Perform timer setup
	*TIMER3_TOP = 0b1111111111;	// Set period
	*TIMER3_IEN = 0x0;	// disable interrupt
	*TIMER3_CMD = 0x1;	// Start timer
	
	// Request interrupts from kernel 
	// TODO: Sett inn SA_INTERRUPT som flag?
	err = request_irq(19, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	if (err) goto release_reg2;
	if (err) goto free_irq1;
	printk("DEBUG: Sound driver Open completed \n");
	return 0;
	
	// Cleanup functions:
	free_irq1: free_irq(19, dev_num);
	release_reg2: release_mem_region((int) TIMER3_BASE, 0x24);
	release_reg1: release_mem_region((int) DAC0_BASE2, 0x24);
	return_err: return err;
}

static int SOUND_release(struct inode *inode, struct file *filp) 
{
	// Release interrupt lines
	free_irq(19, dev_num);
	// Release IO regions
	release_mem_region((int) TIMER3_BASE, 0x24);
	release_mem_region((int) DAC0_BASE2, 0x24);
	return 0;
}

static ssize_t SOUND_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	
	return -1;
}

static ssize_t SOUND_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	
	play(count);
	return 0;
}

static irqreturn_t KERNEL_IRQ_HANDLER(int irq, void *dev_id)
{
	/*
	Interrupt handler which will be triggered by kernel
	*/
	*TIMER3_IFC |= 0x1;	// Clear flag
	*TIMER3_IEN = 0;	// Turn off timer interrupts
	next = false;	// next used in ex2.c as a "timer/clock"
	return IRQ_HANDLED;
}

void playSound(int **waveArr, int sizeWarr, int sizeWave, int *intervalArr, int *durationArr){
	// Loops through sound arrays and calls the method playWave()
	int i;
	for (i = 0; i < sizeWarr; i++) {
		playWave(waveArr[i], sizeWave, *(intervalArr + i), *(durationArr + i));
	}
}

void playWave(int *wavePtr, int size, int interval, int duration){
	// Play a single wave for a given duration
	int timeLeft;
	int i;

	timeLeft=duration;
	while (timeLeft > 0) {
		for (i = 0; i < size; i++) {
			// Feed DAC to play sound, wavePtr is a predefined amplitude array,
			// Volume is used as a multiplier
			*DAC0_CH0DATA = wavePtr[i] * volume;
			*DAC0_CH1DATA = wavePtr[i] * volume;
			setPeriod(interval);	// set TIMER3_TOP to correct interval
			while(next){}	// "clock", wait for interrupt before continuing
			next = true;	// reset next so it enters the while loop above
		}
		timeLeft -= interval * size;
	}
}

void play(size_t melody){
	if(melody == 1){
		playSound(soundTune_1, 7, 16, soundIntervals_1, soundDurations_1);
	}else if(melody == 2){
		playSound(soundTune_2, 7, 16, soundIntervals_2, soundDurations_2);
	}else if (melody == 3){
		playSound(soundTune_5, 6, 4, soundIntervals_5, soundDurations_5);
	}else if (melody == 4){
		playSound(soundTune_4, 6, 4, soundIntervals_4, soundDurations_4);
	}
	else if(melody==5){
		playSound(soundTune_6, 4, 16, soundIntervals_6, soundDurations_6);
		playSound(soundTune_6_5, 4, 16, soundIntervals_6_5, soundDurations_6_5);
		playSound(soundTune_6, 4, 16, soundIntervals_6, soundDurations_6);
		playSound(soundTune_6_5, 4, 16, soundIntervals_6_5, soundDurations_6_5);
		playSound(soundTune_6_5, 4, 16, soundIntervals_6_5, soundDurations_6_5);
		}

}


void setPeriod(uint32_t period){
	*TIMER3_TOP = period;
	*TIMER3_IEN = 1;
}

module_init(SOUND_init);
module_exit(SOUND_cleanup);

MODULE_DESCRIPTION("Driver for the EFM32GG DAC and TIMER peripherals");
MODULE_LICENSE("GPL");

