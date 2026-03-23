#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "jiffies"

static int jiffies_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%lu\n", jiffies);
	return 0;
}

static int jiffies_open(struct inode *inode, struct file *file)
{
	return single_open(file, jiffies_show, NULL);
}

static const struct proc_ops jiffies_proc_ops = {
	.proc_open = jiffies_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int __init jiffies_init(void)
{
	if (!proc_create(PROC_NAME, 0, NULL, &jiffies_proc_ops))
		return -ENOMEM;

	pr_info("/proc/%s created\n", PROC_NAME);
	return 0;
}

static void __exit jiffies_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
	pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(jiffies_init);
module_exit(jiffies_exit);

MODULE_LICENSE("GPL");