#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h> // Memory IO reservation
#include <linux/interrupt.h>  // Used for interrupt tasklets
#include <linux/signal.h>  // Signal usage
#include <linux/sched.h>  // Used for current PID
#include <linux/fs.h>  // Get device number
#include <linux/cdev.h>  // For char dev registration
#include <linux/device.h> // For device creation (c1)

// DECLARE MEMORY LOCATIONS:
// GPIO
#define GPIO_BASE 0x40006000
#define GPIO_PA_BASE 0x40006000
#define GPIO_PB_BASE 0x40006024
#define GPIO_PC_BASE 0x40006048

#define GPIO_PC_CTRL     ((volatile uint32_t*)(GPIO_PC_BASE + 0x00))
#define GPIO_PC_MODEL    ((volatile uint32_t*)(GPIO_PC_BASE + 0x04))
#define GPIO_PC_MODEH    ((volatile uint32_t*)(GPIO_PC_BASE + 0x08))
#define GPIO_PC_DOUT     ((volatile uint32_t*)(GPIO_PC_BASE + 0x0c))
#define GPIO_PC_DOUTSET  ((volatile uint32_t*)(GPIO_PC_BASE + 0x10))
#define GPIO_PC_DOUTCLR  ((volatile uint32_t*)(GPIO_PC_BASE + 0x14))
#define GPIO_PC_DOUTTGL  ((volatile uint32_t*)(GPIO_PC_BASE + 0x18))
#define GPIO_PC_DIN      ((volatile uint32_t*)(GPIO_PC_BASE + 0x1c))
#define GPIO_PC_PINLOCKN ((volatile uint32_t*)(GPIO_PC_BASE + 0x20))

#define GPIO_EXTIPSELL ((volatile uint32_t*)(GPIO_PA_BASE + 0x100))
#define GPIO_EXTIPSELH ((volatile uint32_t*)(GPIO_PA_BASE + 0x104))
#define GPIO_EXTIRISE  ((volatile uint32_t*)(GPIO_PA_BASE + 0x108))
#define GPIO_EXTIFALL  ((volatile uint32_t*)(GPIO_PA_BASE + 0x10c))
#define GPIO_IEN       ((volatile uint32_t*)(GPIO_PA_BASE + 0x110))
#define GPIO_IFC       ((volatile uint32_t*)(GPIO_PA_BASE + 0x11c))
#define GPIO_IF		   ((volatile uint32_t*)(GPIO_BASE + 0x114))

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
 * Declare variables
 */
struct task_struct *task;  // Takes pid number of child process
dev_t *dev_num;
// File operations structure
struct file_operations my_operations;
//Til ut-signal
struct siginfo out_signal;
int PIDCurrent;
// Initialize file operations for driver: TODO: Add more?
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.read = my_read,
	.write = my_write,
};
static struct class* c1 = NULL;
static struct device* d1 = NULL;
// Declare 2nd part of button interrupt handler
DECLARE_TASKLET(button_pushed, transfer_interrupt, 0);

static int __init template_init(void)
{
	int err; // Error checking variable
	const char device_name[] = "GPIO";
	// Device ID, NULL if shared
	*dev_num = 0;
	err = alloc_chrdev_region(dev_num, 0, 1, device_name);  
	// Register char device
	struct cdev *my_cdev = cdev_alloc();
	cdev_init(my_cdev, &fops);
	cdev_add(my_cdev, *dev_num, 1);
	// Make visible driver file in /dev/
	c1 = class_create(THIS_MODULE, device_name);
	if (IS_ERR(c1)) {
		printk("c1 Err \n");
	}
	d1 = device_create(c1, NULL, *dev_num, NULL, device_name);
	if (IS_ERR(d1)) {
		printk("d1 ERR \n");
	}
	// TODO: Allocate IO regions
	//request_mem_region(GPIO_PC_BASE, 0x24, device_name);
	// PortA? 
	// Perform GPIO setup
    *GPIO_PC_DOUT = 0xff; //Set pins to pull up
    *GPIO_PC_MODEL = 0x33333333; // set pins to input
    *GPIO_EXTIPSELL = 0x22222222; // select pins to trigger interrupts
    *GPIO_EXTIFALL = 0xff; // set interrupt to trigger on falling edge
    *GPIO_IEN = 0xff; // enable interrupts
	// Request interrupts from kernel TODO: Flytt til open
	// TODO: Sett inn SA_INTERRUPT som flag?
	err = request_irq(17, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	err = request_irq(18, KERNEL_IRQ_HANDLER, 0, device_name, dev_num);
	printk("DEBUG: Init completed \n");
	return 0;
}

static int my_open (struct inode *inode, struct file *filp) 
{
	// Register client process
	task = current;
	PIDCurrent = current->pid;
	printk("DEBUG: Driver opened! :D \n");
	printk("Driver registered PID: %d \n", PIDCurrent); 
	return 0;
}

static int my_release(struct inode *inode, struct file *filp) 
{
	// TODO: Undo open
	return 0;
}

static ssize_t my_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
	// TODO: Let game retrieve button info
	return 0;
}

static ssize_t my_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
	return 0;
}

static void __exit template_cleanup(void)
{
	// Free device number(s)
	unregister_chrdev_region(0,1);
	 printk("Short life for a small module...\n");
}

static irqreturn_t KERNEL_IRQ_HANDLER(int irq, void *dev_id)
{
	/*
	Interrupt handler which will be triggered by kernel
	*/
	printk("DEBUG: KERNEL_IRQ_HANDLER_CALLED \n");
    *GPIO_IFC = *GPIO_IF;   // Clear interrupt flag
	// Trigger Driver-interrupt (bottom half)
	// TODO: lock maybe? p.276
	tasklet_schedule(&button_pushed);
	return IRQ_HANDLED;
}

void transfer_interrupt(unsigned long unused) 
{
	//Send ready-to-read signal to program (chap. 6)
	//send_sig(SIGIO, task, 0);  //utdatert?
	//TODO: Cleanup
	//SIG_TEST = 44;
	printk("DEBUG: Interrupt tasklet executed \n");
		// TODO: flytt stuff ut herifra
	out_signal.si_signo = SIGUSR1;
    out_signal.si_code = SI_KERNEL;
	out_signal.si_int = SIGUSR1;  
	send_sig_info(SIGUSR1, &out_signal, task);
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

