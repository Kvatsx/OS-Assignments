#include <linux/init.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

static char MessageBuffer[256];
static int BufferSize;
static int fdOpens = 0;
static char Key_user[16];
static int Flag = 0;
static char EncryptedMessage[512];

static int dec_open(struct inode*, struct file*);
static ssize_t dec_read(struct file *, char *, size_t, loff_t *);
static ssize_t dec_write(struct file *, const char *, size_t, loff_t *);
static int dec_release(struct inode *, struct file *);

static struct file_operations dec_file_operations = {
    .open = dec_open,
    .read = dec_read,
    .write = dec_write,
    .release = dec_release,
};

int dec_dev_module_init(void)
{
    printk("Registering Device.\n");
    register_chrdev(230, "Decryption Device", &dec_file_operations);
    return 0;
}

void dec_dev_module_exit(void)
{
    printk("Unregistering Device.\n");
    unregister_chrdev(230, "Decryption Device");
}

static int dec_open(struct inode *inodep, struct file *filep)
{
    fdOpens++;
    printk("File opened %d times.\n", fdOpens);
    return 0;
}

static ssize_t dec_read(struct file *filep, char *InputMessage, size_t len, loff_t *offset)
{
    if ( copy_to_user(InputMessage, EncryptedMessage, strlen(EncryptedMessage)) != 0 )
    {
        printk("Failed to send characters to the user.\n");
        return -EFAULT;
    }
    // if ( copy_to_user(InputMessage, MessageBuffer, BufferSize) != 0 )
    // {
    //     printk("Failed to send characters to the user.\n");
    //     return -EFAULT;
    // }
    printk("DEC Message recv\n");
    BufferSize = 0;
    return 0;
}

static ssize_t dec_write(struct file *filep, const char *InputMessage, size_t len, loff_t *offset)
{   
    if ( Flag == 0 )
    {   
        printk("DEC Recieved Key");
        int i;
        for ( i=0; i<16; i++ )
        {
            Key_user[i] = InputMessage[i];
        }
        Flag = 1;
        BufferSize = 16;
        return len;
    }
    else if ( Flag == 1 )
    {
        printk("DEC Recieved Message");
        int iter;
        for ( iter = 0; iter < len; iter++ )
        {
            EncryptedMessage[iter] = (int)InputMessage[iter]^(int)Key_user[iter%16];
            Key_user[iter%16] = InputMessage[iter];
        }
        EncryptedMessage[iter] = '\0';
        BufferSize = strlen(EncryptedMessage);

        return len;
    }
    else
    {
        printk("Flag > 1\n");
        return len;
    }

    // sprintf(MessageBuffer, "%s", InputMessage);
    // BufferSize = strlen(MessageBuffer);
    // printk("Message To Write: %s\n", InputMessage);
    // return len;
}

static int dec_release(struct inode *inodep, struct file *filep)
{
    printk("Device successfully closed.\n");
    return 0;
}

module_init(dec_dev_module_init);
module_exit(dec_dev_module_exit);