#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/syscall_wrapper.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/init_task.h>
#include <linux/fs.h>

#ifndef __FTRACE_HOOKING
#define __FTRACE_HOOKING

//ftrace Func.'s data
struct ftrace_data{
    pid_t pid;
    char *program_name;
    char file_name[256];
    ssize_t read_byte, write_byte;
    int open_count, close_count, read_count, write_count, lseek_count;
};

extern struct ftrace_data ft_data;

#endif