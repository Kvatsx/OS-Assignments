#include <linux/kernel.h>

asmlinkage long sys_helloworld(void){
	printk("hello world\n");
	return 0;
}
