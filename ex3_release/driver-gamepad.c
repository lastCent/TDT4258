#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h> 		// Memory IO reservation
#include <linux/interrupt.h>		// Used for interrupt tasklets
#include <linux/signal.h>  		// Signal usage
#include <linux/sched.h>  		// Used for current PID
#include <linux/fs.h>  			// Get device number
#include <linux/cdev.h>  		// For char dev registration
#include <linux/device.h> 		// For device creation (c1)
#include <asm/uaccess.h>  		// Userspace read/write

/*
 * DECLARE MEMORY LOCATIONS:
 *-----------------------------------------------------------------------------
 */

#define GPIO_BASE 	0x40006000
#define GPIO_PC_BASE 	0x40006048

#define GPIO_PC_MODEL	((volatile uint32_t*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_DOUT	((volatile uint32_t*)(GPIO_PC_BASE + 0x0c))
#define GPIO_PC_DIN	((volatile uint32_t*)(GPIO_PC_BASE + 0x1c))

#define GPIO_EXTIPSELL	((volatile uint32_t*)(GPIO_BASE + 0x100))
#define GPIO_EXTIFALL	((volatile uint32_t*)(GPIO_BASE + 0x10c))
#define GPIO_IEN	((volatile uint32_t*)(GPIO_BASE + 0x110))
#define GPIO_IF		((volatile uint32_t*)(GPIO_BASE + 0x114))
#define GPIO_IFC	((volatile uint32_t*)(GPIO_BASE + 0x11c))


/*
 * Declare function prototypes
 *-----------------------------------------------------------------------------
 */

static int	gamepad_open (struct inode *, struct file *);
static int	gamepad_release(struct inode *, struct file *);
static ssize_t	gamepad_read(struct file *, char *, size_t, loff_t *);
static ssize_t	gamepad_write(struct file *, const char *, size_t, loff_t *);
void 		transfer_interrupt(unsigned long);
static 		irqreturn_t KERNEL_IRQ_HANDLER(int, void*);

/*
 * Global variables:
 *----------------------------------------------------------------------------- 
 */

const char device_name[] = "Gamepad";	// Name of driver
dev_t *dev_num;  			// Device ID, NULL if shared
static int majorNumber;			// Assigned major number
struct task_struct *task;  		// Takes pid number of child process
struct siginfo out_signal = {};  	// Signal which is sent to userspace
static struct class* c1;  		// Device class
static struct device* d1;  		// Device file

// Supported file operations struct
const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = gamepad_open,
	.release = gamepad_release,
	.read = gamepad_read,
	.write = gamepad_write,
};

// Declare 2nd part of interrupt handler
DECLARE_TASKLET(button_pushed, transfer_interrupt, 0);

static int __init gamepad_init(void)
{
	int err = 0;  			// Error checking variable
	struct cdev *my_cdev;		// Kernel char device structure

	// Allocate major number (id for device)
	majorNumber = register_chrdev(0, device_name, &fops);
	*dev_num = MKDEV(majorNumber, 0);
	if (majorNumber<0) goto return_err;
	
	// Register char device
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &fops);
	err = cdev_add(my_cdev, MKDEV(majorNumber, 0), 1);
	if (err) goto unregister_dev;
	
	// Make visible driver file in /dev/
	c1 = class_create(THIS_MODULE, device_name);
	if (IS_ERR(c1)) 
	{
		err = -1;
		goto unregister_dev;
	}
	d1 = device_create(c1, NULL, MKDEV(majorNumber, 0), NULL, device_name);
	if (IS_ERR(d1)) 
	{
		err = -1;
		goto destroy_class;
	}
	
	// Define signal which is sent to userspace
	out_signal.si_signo = SIGUSR1;
	out_signal.si_code = SI_KERNEL;
	out_signal.si_int = SIGUSR1; 
	return 0;
	
	// Cleanup functions:
	destroy_class: class_destroy(c1);
	unregister_dev: unregister_chrdev_region(MKDEV(majorNumber, 0), 1);
	return_err: return err;
}

static void __exit gamepad_cleanup(void)
{
	// Remove device file
	device_destroy(c1, MKDEV(majorNumber, 0));
	
	// Remove class
	class_destroy(c1);
	
	// Free device (number)
	unregister_chrdev_region(MKDEV(majorNumber, 0), 1);
}

static int gamepad_open (struct inode *inode, struct file *filp) 
{
	int err;  	// Error checking variable
	void* region;  	// request_mem_region error checking
	
	// Register client process
	task = current;
	
	// Allocate IO regions
	region = request_mem_region(GPIO_PC_BASE, 0x24, device_name);
	if (region == NULL) 
	{
		err = -1;
		goto return_err;
	}
	region = request_mem_region((int) GPIO_EXTIPSELL, 0x20, device_name); 
	if (region ==NULL) 
	{
		err = -1;
		goto release_reg1;
	} 
	
	// Perform GPIO setup
	*GPIO_PC_DOUT = 0xff; 			// Set pins to pull up
    	*GPIO_PC_MODEL = 0x33333333; 		// set pins to input
    	*GPIO_EXTIPSELL = 0x22222222; 		// select pins to trigger interrupts
    	*GPIO_EXTIFALL = 0xff; 			// set interrupt to trigger on falling edge
    	*GPIO_IEN = 0xff; 			// enable interrupts
	
	// Request interrupts from kernel 
	err = request_irq(17, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	if (err) goto release_reg2;
	err = request_irq(18, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	if (err) goto free_irq1;
	return 0;
	
	// Cleanup functions:
	free_irq1: free_irq(17, dev_num); free_irq(18, dev_num);
	release_reg2: release_mem_region((int) GPIO_EXTIPSELL, 0x20);
		*GPIO_PC_DOUT = 0; *GPIO_PC_MODEL = 0; *GPIO_EXTIPSELL = 0;
        	*GPIO_EXTIFALL = 0; *GPIO_IEN = 0;
	release_reg1: release_mem_region((int) GPIO_PC_BASE, 0x24);
	return_err: return err;
}

static int gamepad_release(struct inode *inode, struct file *filp) 
{
	// Undo GPIO setup
	*GPIO_PC_DOUT = 0;
	*GPIO_PC_MODEL = 0;
	*GPIO_EXTIPSELL = 0;
	*GPIO_EXTIFALL = 0;
	*GPIO_IEN = 0;

	// Release interrupt lines
	free_irq(17, dev_num);
	free_irq(18, dev_num);
	
	// Release IO regions
	release_mem_region((int) GPIO_EXTIPSELL, 0x20);
	release_mem_region((int) GPIO_PC_BASE, 0x24);
	return 0;
}

static ssize_t gamepad_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	// Read GPIO buttons, put copy into userspace buffer
	uint32_t outVal;
	outVal = *GPIO_PC_DIN;
	copy_to_user(buff, &outVal, sizeof(outVal)); 
	return 0;
}

static ssize_t gamepad_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	// Does nothing
	return -1;
}

static irqreturn_t KERNEL_IRQ_HANDLER(int irq, void *dev_id)
{
	
	// Interrupt handler which will be triggered by kernel
	*GPIO_IFC = *GPIO_IF;   		// Clear interrupt flag
	tasklet_schedule(&button_pushed);	// Queue driver-interrupt (bottom half)
	return IRQ_HANDLED;
}

void transfer_interrupt(unsigned long unused) 
{
	// Send the signal to client program
	send_sig_info(SIGUSR1, &out_signal, task);
}

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("Driver for the custom NTNU gamepad device");
MODULE_LICENSE("GPL");

