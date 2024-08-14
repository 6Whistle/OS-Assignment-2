#include "ftracehooking.h"

#define __NR_ftrace 336     //System Call Number

asmlinkage int (*real_ftrace)(const struct pt_regs *regs);      //Save real ftrace Func.

void **syscall_table;       //System Call Table

struct ftrace_data ft_data; //Store data(ftrace), extern variable
EXPORT_SYMBOL(ft_data);

//hijack ftrace System Call
static asmlinkage int ftrace(const struct pt_regs *regs){
    pid_t pid = real_ftrace(regs);      //Get pid from real_ftrace

    //Start Point
    if(pid != 0){
        //Get program name from task
        struct task_struct *findtask = pid_task(find_vpid(pid), PIDTYPE_PID);
        memset((void *)&ft_data, 0, sizeof(ft_data));

        //Store pid and program name
        ft_data.pid = pid;
        ft_data.program_name = findtask->comm;
    
        printk(KERN_INFO "OS Assignment 2 ftrace [%d] Start\n", pid);
    }
    else{       //End Point(Print about ft_data)
        printk(KERN_INFO "[2018202046] /%s file[%s] start [x] read - %ld / written - %ld\n",
        ft_data.program_name, ft_data.file_name, ft_data.read_byte, ft_data.write_byte);
        printk(KERN_INFO "open[%d] close[%d] read[%d] write[%d] lseek[%d]\n",
        ft_data.open_count, ft_data.close_count, ft_data.read_count, ft_data.write_count, ft_data.lseek_count);
        printk(KERN_INFO "OS Assignment 2 ftrace [%d] End\n", ft_data.pid);
    }
    
    return pid;
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
static int __init ftracehooking_init(void){
    syscall_table = (void**) kallsyms_lookup_name("sys_call_table");
    make_rw(syscall_table);
    real_ftrace = syscall_table[__NR_ftrace];
    syscall_table[__NR_ftrace] = ftrace;
    make_ro(syscall_table);
    return 0;
}

//Module exit Func.
static void __exit ftracehooking_exit(void){
    make_rw(syscall_table);
    syscall_table[__NR_ftrace] = real_ftrace;
    make_ro(syscall_table);
}

module_init(ftracehooking_init);
module_exit(ftracehooking_exit);
MODULE_LICENSE("GPL");
