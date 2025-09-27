#include <linux/init.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/printk.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Assignment 3: tick/time and elapsed via jiffies");
MODULE_VERSION("1.0");

static unsigned long start_jiffies;
static struct timespec64 start_ts;

static void get_current_hms(int *hh, int *mm, int *ss)
{
    struct timespec64 ts;
    long secs;
    ktime_get_real_ts64(&ts);
    secs = (long)(ts.tv_sec % 86400);
    if (secs < 0) secs += 86400;
    *hh = (int)(secs / 3600);
    secs %= 3600;
    *mm = (int)(secs / 60);
    *ss = (int)(secs % 60);
}

static int __init lab3_init(void)
{
    int hh, mm, ss;
    unsigned int tick_ms;
    start_jiffies = jiffies;
    ktime_get_real_ts64(&start_ts);
    tick_ms = jiffies_to_msecs(1);
    get_current_hms(&hh, &mm, &ss);
    printk(KERN_INFO "hello\n");
    printk(KERN_INFO "tick interval: %u ms\n", tick_ms);
    printk(KERN_INFO "Current time is: %02d:%02d:%02d\n", hh, mm, ss);
    return 0;
}

static void __exit lab3_exit(void)
{
    unsigned long end_jiffies = jiffies;
    unsigned long delta_j = end_jiffies - start_jiffies;
    unsigned int elapsed_ms = jiffies_to_msecs(delta_j);
    int hh, mm, ss;
    get_current_hms(&hh, &mm, &ss);
    printk(KERN_INFO "goodbye\n");
    printk(KERN_INFO "Elapsed since insert: %u ms\n", elapsed_ms);
    printk(KERN_INFO "Current time is: %02d:%02d:%02d\n", hh, mm, ss);
}

module_init(lab3_init);
module_exit(lab3_exit);
