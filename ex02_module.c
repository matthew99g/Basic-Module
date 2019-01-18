#include <linux/init.h>				/* Needed for macros */
#include <linux/module.h>			/* Needed by all modules */
#include <linux/moduleparam.h>		/* Needed to take parameters from shell */
#include <linux/stat.h>
#include <linux/kernel.h>			/* KERN_INFO */
#include <linux/fs.h>				/* KERNEL MODE FS FOR CHAR DEVICE DRIVER*/
#include <asm/uaccess.h>			/* For sending kernel data to user */

#define DRIVER_AUTHOR 	"Matthew Todd Geiger <matthewgeiger99@gmail.com>"
#define DRIVER_DESC		"Sample Test Driver"

/* Example variable input via shell terminal */
static int iData = 0;
static int major = 0;
static int iDeviceOpen = 0;

static char szBuffer[80];
static char *szPtr;

//
// Define IO Functions(File Operations)
int ex01_open(struct inode *inode, struct file *file) {
	if(iDeviceOpen) {
		printk(KERN_ALERT "Device already open!");
		return -EBUSY;
	}

	iDeviceOpen++;

	sprintf(szBuffer, "Test Message!\n");
	szPtr = szBuffer;

	try_module_get(THIS_MODULE);

	printk(KERN_INFO "IN %s", __FUNCTION__);
	return 0;
}
ssize_t
ex01_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
        /*
         * Number of bytes actually written to the buffer 
         */
        int bytes_read = 0;
        /*
         * If we're at the end of the message, 
         * return 0 signifying end of file 
         */
        if (*szPtr == 0)
                return 0;
        /* 
         * Actually put the data into the buffer 
         */
        while (length && *szPtr) {
                /* 
                 * The buffer is in the user data segment, not the kernel 
                 * segment so "*" assignment won't work.  We have to use 
                 * put_user which copies data from the kernel data segment to
                 * the user data segment. 
                 */
                put_user(*(szPtr++), buffer++);
                length = length - 1;
                bytes_read = bytes_read + 1;
        }
        /* 
         * Most read functions return the number of bytes put into the buffer
         */
        return bytes_read;
}
/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
ssize_t
ex01_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
        printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
        return EINVAL;
}
int ex01_close(struct inode *inode, struct file *file){
	if(iDeviceOpen <= 0) {
		printk(KERN_ALERT "Device already closed!");
		return -EBUSY;
	}

	iDeviceOpen--;

	printk(KERN_INFO "IN %s", __FUNCTION__);
	
	module_put(THIS_MODULE);
	
	return 0;
}

struct file_operations ex01_file_operations = {
	.owner = THIS_MODULE,
	.open = ex01_open,
	.read = ex01_read,
	.write = ex01_write,
	.release = ex01_close,
};

/* Grab command line param */
module_param(iData, int, 0644);

// 
//	Init Function 
static int __init
ex01_module_init(void)
{
	printk(KERN_INFO "ex01_module: Indside the %s function | %d\n", __FUNCTION__, iData);

	/* Register CHAR DEVICE with kernel */
	major = register_chrdev(240,
					"Simple CHAR Driver",
					&ex01_file_operations);
	if(major < 0) {
		printk(KERN_ALERT "Failed to register device driver\n");
		return major;
	}

	return 0;
}

//
// Exit Function
static void __exit
ex01_module_exit(void)
{
	printk(KERN_INFO "ex01_module: Exiting");

	/* Unregister CHAR DEVICE with kernel */
	unregister_chrdev(240, "Simple Char Driver");
}

/* Set init and exit module functions */
module_init(ex01_module_init);
module_exit(ex01_module_exit);

MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);