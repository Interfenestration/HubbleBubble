#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>

int (* sys_qqservice_impl)(int, void *, int);
int (* sys_qqservice_named_attach_impl) (char *);
int (* sys_qqservice_named_op_impl) (int, char *)

EXPORT_SYMBOL(sys_qqservice_impl);
EXPORT_SYMBOL(sys_qqservice_named_attach_impl);
EXPORT_SYMBOL(sys_qqservice_named_op_impl)

asmlinkage int sys_qqservice(int op, void * msg, int size) {
    if(sys_qqservice_impl == 0)
        return -1;
    else
        return sys_qqservice_impl(op, msg, size);
}

asmlinkage int sys_qqservice_named_attach(void * name) {
    if(sys_qqservice_named_attach_impl == 0)
        return -1;
    else
        return sys_qqservice_named_attach_impl(name);
}

asmlinkage int sys_qqservice_named_op(int op, int queueId) {
    if(sys_qqservice_named_op_impl == 0)
        return -1;
    else
        return sys_qqservice_named_op_impl(op, queueId);
}
