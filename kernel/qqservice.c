#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>

int (* sys_qqmodule) (int, int, char *, int);
int (* sys_qqmodule_named_attach) (char *, int, pid_t);
int (* sys_qqmodule_named) (int, int, pid_t);

asmlinkage int sys_qqservice(int op, int queue_id, char * msg, int size) {
    if(sys_qqmodule == 0) {
        printk("qqservice: failed to access sys_qqmodule\n");
        return -1;
    }
    else
        return sys_qqmodule(op, queue_id, msg, size);
}

asmlinkage int sys_qqservice_named_attach(char * name, int size, pid_t pid) {
    if(sys_qqmodule_named_attach == 0) {
        printk("qqservice: failed to access sys_qqmodule_named_attach\n");
        return -1;
    }
    else
        return sys_qqmodule_named_attach(name, size, pid);
}

asmlinkage int sys_qqservice_named(int op, int queue_id, pid_t pid) {
    if(sys_qqmodule_named == 0) {
        printk("qqservice: failed to access sys_qqmodule_named\n");
        return -1;
    }
    else
        return sys_qqmodule_named(op, queue_id, pid);
}

EXPORT_SYMBOL(sys_qqmodule);
EXPORT_SYMBOL(sys_qqmodule_named_attach);
EXPORT_SYMBOL(sys_qqmodule_named);
