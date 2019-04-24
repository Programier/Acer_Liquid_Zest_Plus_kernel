/*
 *  Copyright (c) 2015 Acer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/export.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define FINGERPRINT_LEN		8

static struct kobject* sys_info;
static char fp[FINGERPRINT_LEN] = {'\xdc', '\x00', '\x85', '\xbb', '\xc6', '\x3a', '\xd6', '\x2f'};

static ssize_t fp_read(struct file *filp, struct kobject *kobj,
			struct bin_attribute *bin_attr,
			char *buf, loff_t off, size_t count)
{
	return memory_read_from_buffer(buf, count, &off, fp, sizeof(fp));
}

static struct bin_attribute fp_attr = {
	.attr = {
		.name = "fp",
		.mode = S_IRUGO,
	},
	.size = sizeof(fp),
	.read = fp_read,
};

static int __init acer_sec_init(void)
{
	int error;

	sys_info = kobject_create_and_add("sys_info", NULL);
	if (sys_info == NULL) {
		pr_err("Failed to create sys_info kobject\n");
		return -ENOMEM;
	}

	if ((error = sysfs_create_bin_file(sys_info, &fp_attr))) {
		pr_err("Failed to create fp\n");
		goto err;
	}

	return 0;

err:
	kobject_put(sys_info);
	return error;
}

device_initcall(acer_sec_init);
