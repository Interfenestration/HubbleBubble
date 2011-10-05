#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>

int (* sys_qqservice_impl)(int, void *, int);
int (* sys_qqservice_named_attach_impl) (char *);
int (* sys_qqservice_named_op_impl) (int, char *)

EXPORT_SYMBOL(sys_qqmodule);
EXPORT_SYMBOL(sys_qqmodule_named_attach);
EXPORT_SYMBOL(sys_qqmodule_named)

asmlinkage int sys_qqservice(int op, void * msg, int size) {
    if(sys_qqservice_impl == 0)
        return -1;
    else
        return sys_qqmodule(op, msg, size);
}

asmlinkage int sys_qqservice_named_attach(void * name) {
    if(sys_qqservice_named_attach_impl == 0)
        return -1;
    else
        return sys_qqmodule_named_attach(name);
}

asmlinkage int sys_qqservice_named(int op, int queueId) {
    if(sys_qqservice_named_op_impl == 0)
        return -1;
    else
        return sys_qqmodule_named(op, queueId);
}
