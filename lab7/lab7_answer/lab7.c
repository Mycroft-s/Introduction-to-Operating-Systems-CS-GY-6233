#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuyin Lin");
MODULE_DESCRIPTION("Assignment 7 - misc char device");
MODULE_VERSION("1.0");

static unsigned long start_jiffies;
static int already_read = 0;

static int lab7_open(struct inode *inode, struct file *file)
{
    start_jiffies = jiffies;
    already_read = 0;
    return 0;
}

static ssize_t lab7_read(struct file *file, char __user *buf,
                         size_t count, loff_t *ppos)
{
    char kbuf[128];
    int len;
    unsigned long diff = jiffies - start_jiffies;
    unsigned long ms = jiffies_to_msecs(diff);
    pid_t ppid = current->parent ? current->parent->pid : -1;

    if (already_read)
        return 0;

    len = snprintf(kbuf, sizeof(kbuf),
                   "elapsed: %lu ms\nppid: %d\n",
                   ms, ppid);

    if (len > count)
        len = count;

    if (copy_to_user(buf, kbuf, len))
        return -EFAULT;

    already_read = 1;
    return len;
}

static const struct file_operations lab7_fops = {
    .owner = THIS_MODULE,
    .open  = lab7_open,
    .read  = lab7_read,
};

static struct miscdevice lab7_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "lab7dev",
    .fops  = &lab7_fops,
};

static int __init lab7_init(void)
{
    return misc_register(&lab7_dev);
}

static void __exit lab7_exit(void)
{
    misc_deregister(&lab7_dev);
}

module_init(lab7_init);
module_exit(lab7_exit);
