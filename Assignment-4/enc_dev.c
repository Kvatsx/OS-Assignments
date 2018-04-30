#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/random.h>


static char MessageBuffer[512];
static int BufferSize;
static int fdOpens = 0;
static char Key_user[16];
static int Flag = 0;
static char EncryptedMessage[512];

static int enc_open(struct inode*, struct file*);
static ssize_t enc_write(struct file *, const char *, size_t, loff_t *);
static int enc_release(struct inode *, struct file *);
static ssize_t enc_read(struct file *filep, char *InputMessage, size_t len, loff_t *offset);

static struct file_operations enc_file_operations = {
    .open = enc_open,
    .write = enc_write,
    .release = enc_release,
    .read = enc_read
};

int enc_dev_module_init(void)
{
    printk("Registering Device.\n");
    register_chrdev(220, "Encryption Device", &enc_file_operations);
    return 0;
}

void enc_dev_module_exit(void)
{
    printk("Unregistering Device.\n");
    unregister_chrdev(220, "Encryption Device");
}

static int enc_open(struct inode *inodep, struct file *filep)
{
    fdOpens++;
    printk("File opened %d times.\n", fdOpens);
    return 0;
}

static ssize_t enc_write(struct file *filep, const char *InputMessage, size_t len, loff_t *offset)
{
	if ( Flag == 0 )
	{	
		int i;
		for ( i=0; i<16; i++ )
		{
			Key_user[i] = InputMessage[i];
		}
		Flag = 1;
		BufferSize = 16;
		printk("ENC Length: %d\n", strlen(Key_user));
		return len;
	}
	else if ( Flag == 1 )
	{
		printk("Entered Mssg: %s\n", InputMessage);
		int iter;
		for ( iter = 0; iter < strlen(InputMessage); iter++ )
		{
			EncryptedMessage[iter] = (char)((int)InputMessage[iter]^(int)Key_user[iter%16]);
			Key_user[iter%16] = EncryptedMessage[iter];
			// printk("char: %c\n", InputMessage[iter]);
		}
		EncryptedMessage[iter] = '\0';
		BufferSize = strlen(EncryptedMessage);
		printk("ENC Length of EncryptedMessage: %d\n", strlen(EncryptedMessage));
	    return len;
	}
	else
	{
		printk("Flag > 1\n");
		return len;
	}
}

static ssize_t enc_read(struct file *filep, char *InputMessage, size_t len, loff_t *offset)
{
	// if ( Flag == 0 )
	// {
	// 	if ( copy_to_user(InputMessage, Key_user, 16) != 0 )
 //    	{
 //        	printk("Failed to send characters to the user.\n");
 //        	return -EFAULT;
 //    	}
 //    	Flag = 1;
	// }
	// else if ( Flag == 1 )
	// {
		if ( copy_to_user(InputMessage, EncryptedMessage, strlen(EncryptedMessage)) != 0 )
    	{
        	printk("Failed to send characters to the user.\n");
        	return -EFAULT;
    	}
    	// Flag = 0;
	// }

    printk("ENC Message recv\n");
    BufferSize = 0;
    return 0;
}

static int enc_release(struct inode *inodep, struct file *filep)
{
    printk("Device successfully closed.\n");
    return 0;
}

module_init(enc_dev_module_init);
module_exit(enc_dev_module_exit);