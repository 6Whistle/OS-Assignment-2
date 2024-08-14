#include "ftracehooking.h"

//System Call Num
#define __NR_open 2
#define __NR_close 3
#define __NR_read 0
#define __NR_write 1
#define __NR_lseek 8

//Original Func.
asmlinkage long (*real_open)(const struct pt_regs *regs);
asmlinkage long (*real_close)(const struct pt_regs *regs);
asmlinkage long (*real_read)(const struct pt_regs *regs), (*real_write)(const struct pt_regs *regs);
asmlinkage long (*real_lseek)(const struct pt_regs *regs);

void **syscall_table;       //System call table
extern struct ftrace_data ft_data;      //store at ftrace's data

//hijack open's System call
static asmlinkage long ftrace_open(const struct pt_regs *regs){
    copy_from_user((ft_data.file_name), (const char *)regs->di, sizeof(ft_data.file_name));     //Copy user space's file name data -> kernel space
    ft_data.open_count++;

    return real_open(regs);
}

//hijack close's System call
static asmlinkage long ftrace_close(const struct pt_regs *regs){
    ft_data.close_count++;

    return real_close(regs);
}

//hijack read's System call
static asmlinkage long ftrace_read(const struct pt_regs *regs){
    ssize_t size = real_read(regs);
    ft_data.read_count++;
    ft_data.read_byte += size;
        
    return size;
}

//hijack write's System call
static asmlinkage long ftrace_write(const struct pt_regs *regs){
    ssize_t size = real_write(regs);
    ft_data.write_count++;
    ft_data.write_byte += size;

    return size;
}

//hijack lseek's System call
static asmlinkage long ftrace_lseek(const struct pt_regs *regs){
    ft_data.lseek_count++;

    return real_lseek(regs);
}

//systemcall table get permission(Read Write)
void make_rw(void *addr){
    unsigned int level;
    pte_t *pte = lookup_address((u64)addr, &level);
    if(pte->pte &~ _PAGE_RW)
            pte->pte |= _PAGE_RW;
}

//systemcall table get permission(Read Only)
void make_ro(void *addr){
        unsigned int level;
        pte_t *pte = lookup_address((u64)addr, &level);
    
        pte->pte = pte->pte &~ _PAGE_RW;
}

//Module initialize Func.
static int __init iotracehooking_init(void){
    syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);
    real_open = syscall_table[__NR_open];
    real_close = syscall_table[__NR_close];
    real_read = syscall_table[__NR_read];
    real_write = syscall_table[__NR_write];
    real_lseek = syscall_table[__NR_lseek];

    syscall_table[__NR_open] = ftrace_open;
    syscall_table[__NR_close] = ftrace_close;
    syscall_table[__NR_read] = ftrace_read;
    syscall_table[__NR_write] = ftrace_write;
    syscall_table[__NR_lseek] = ftrace_lseek;
    make_ro(syscall_table);

    return 0;
}

//Module exit Func.
static void __exit iotracehooking_exit(void){
    make_rw(syscall_table);
    syscall_table[__NR_open] = (void *)real_open;
    syscall_table[__NR_close] = (void *)real_close;
    syscall_table[__NR_read] = (void *)real_read;
    syscall_table[__NR_write] = (void *)real_write;
    syscall_table[__NR_lseek] = (void *)real_lseek;
    make_rw(syscall_table);
}

module_init(iotracehooking_init);
module_exit(iotracehooking_exit);
MODULE_LICENSE("GPL");
