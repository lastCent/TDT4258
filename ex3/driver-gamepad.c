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

/*
 * DECLARE MEMORY LOCATIONS:
 */
#define GPIO_BASE 0x40006000
#define GPIO_PC_BASE 0x40006048

#define GPIO_PC_MODEL    ((volatile uint32_t*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_DOUT     ((volatile uint32_t*)(GPIO_PC_BASE + 0x0c))
#define GPIO_PC_DIN      ((volatile uint32_t*)(GPIO_PC_BASE + 0x1c))

#define GPIO_EXTIPSELL ((volatile uint32_t*)(GPIO_BASE + 0x100))
#define GPIO_EXTIFALL  ((volatile uint32_t*)(GPIO_BASE + 0x10c))
#define GPIO_IEN       ((volatile uint32_t*)(GPIO_BASE + 0x110))
#define GPIO_IF		   ((volatile uint32_t*)(GPIO_BASE + 0x114))
#define GPIO_IFC       ((volatile uint32_t*)(GPIO_BASE + 0x11c))


/*
 * Declare function prototypes
 */
static int my_open (struct inode *, struct file *);
static int my_release(struct inode *, struct file *);
static ssize_t my_read(struct file *, char *, size_t, loff_t *);
static ssize_t my_write(struct file *, const char *, size_t, loff_t *);
void transfer_interrupt(unsigned long);
static irqreturn_t KERNEL_IRQ_HANDLER(int, void*);

/*
 * Global variables: 
 */
const char device_name[] = "GPIO";  // Name of driver
//int *dev_num;
dev_t *dev_num;  // Device ID, NULL if shared
struct task_struct *task;  // Takes pid number of child process
struct siginfo out_signal = {};  // Signal which is sent to userspace

// Supported file operations struct
const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
};
// Declare 2nd part of button interrupt handler
DECLARE_TASKLET(button_pushed, transfer_interrupt, 0);

static int __init template_init(void)
{
	int err;  // Error checking variable
	struct cdev *my_cdev;  // Kernel char device structure
	static struct class* c1;
	static struct device* d1;
	// Allocate major number (id for device)
	*dev_num = 0;  // Assign temporary value to please compiler
	err = alloc_chrdev_region(dev_num, 0, 1, device_name); 
	// Register char device
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &fops);
	err = cdev_add(my_cdev, *dev_num, 1);
	// Make visible driver file in /dev/
	c1 = class_create(THIS_MODULE, device_name);
	if (IS_ERR(c1)) {
		printk("c1 Err \n");
	}
	d1 = device_create(c1, NULL, *dev_num, NULL, device_name);
	if (IS_ERR(d1)) {
		printk("d1 ERR \n");
	}
	// Define signal which is sent to userspace
	out_signal.si_signo = SIGUSR1;
    out_signal.si_code = SI_KERNEL;
	out_signal.si_int = SIGUSR1; 
	return 0;
}

static void __exit template_cleanup(void)
{
	// Remove 
	
	// Free device number(s)
	unregister_chrdev_region(0,1);
	printk("Short life for a small module...\n");
}

static int my_open (struct inode *inode, struct file *filp) 
{
	int err;  // Error checking variable
	void* region;  // request_mem_region error checking
	// Register client process
	task = current;
	// Allocate IO regions
	region = request_mem_region(GPIO_PC_BASE, 0x24, device_name);
	printk("DEBUG: Driver opened! :D \n");
	if (region == NULL) {
		printk("ERROR, region 1 failed \n");
	}
	region = request_mem_region((int) GPIO_EXTIPSELL, 0x20, device_name); 
	if (region ==NULL) {
		printk("ERROR, region 2, failed \n");
	} 
	// Perform GPIO setup
	*GPIO_PC_DOUT = 0xff; 			//Set pins to pull up
    *GPIO_PC_MODEL = 0x33333333; 	// set pins to input
    *GPIO_EXTIPSELL = 0x22222222; 	// select pins to trigger interrupts
    *GPIO_EXTIFALL = 0xff; 			// set interrupt to trigger on falling edge
    *GPIO_IEN = 0xff; 				// enable interrupts
	// Request interrupts from kernel 
	// TODO: Sett inn SA_INTERRUPT som flag?
	err = request_irq(17, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	err = request_irq(18, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	printk("DEBUG: Open completed \n");
	return 0;
}

static int my_release(struct inode *inode, struct file *filp) 
{
	// Release interrupt lines
	free_irq(17, dev_num);
	free_irq(18, dev_num);
	// Release IO regions
	release_mem_region((int) GPIO_EXTIPSELL, 0x20);
	release_mem_region((int) GPIO_PC_BASE, 0x24);
	return 0;
}

static ssize_t my_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	// Read GPIO buttons, put copy into userspace buffer
	uint32_t outVal;
	outVal = ~*GPIO_PC_DIN;
	copy_to_user(buff, &outVal, sizeof(outVal)); 
	return 0;
}

static ssize_t my_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	return -1;
}

static irqreturn_t KERNEL_IRQ_HANDLER(int irq, void *dev_id)
{
	/*
	Interrupt handler which will be triggered by kernel
	*/
	printk("DEBUG: KERNEL_IRQ_HANDLER_CALLED \n");
    *GPIO_IFC = *GPIO_IF;   // Clear interrupt flag
	tasklet_schedule(&button_pushed);// Queue driver-interrupt (bottom half)
	return IRQ_HANDLED;
}

void transfer_interrupt(unsigned long unused) 
{
	printk("DEBUG: Interrupt tasklet executed \n"); 
	send_sig_info(SIGUSR1, &out_signal, task);
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

