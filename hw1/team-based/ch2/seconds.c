#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "seconds"

static unsigned long start_jiffies;

static int seconds_show(struct seq_file *m, void *v)
{
	unsigned long elapsed = jiffies - start_jiffies;
	unsigned long seconds = elapsed / HZ;

	seq_printf(m, "%lu\n", seconds);
	return 0;
}

static int seconds_open(struct inode *inode, struct file *file)
{
	return single_open(file, seconds_show, NULL);
}

static const struct proc_ops seconds_proc_ops = {
	.proc_open = seconds_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int __init seconds_init(void)
{
	start_jiffies = jiffies;

	if (!proc_create(PROC_NAME, 0, NULL, &seconds_proc_ops))
		return -ENOMEM;

	pr_info("/proc/%s created\n", PROC_NAME);
	return 0;
}

static void __exit seconds_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(seconds_init);
module_exit(seconds_exit);

MODULE_LICENSE("GPL");