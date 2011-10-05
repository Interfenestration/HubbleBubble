#include <linux/module.h>

static int __init qqmodule_init(void) {
    return 0;
}

static void __exit qqmodule_exit(void) {

}

int sys_qqmodule(int op, void * msg, int size) {
    return 0;
}

int sys_qqmodule_named_attach(void * name) {
    return 0;
}

int sys_qqmodule_named(int op, int queueId) {
    return 0;
}

module_init(qqmodule_init);
module_exit(qqmodule_exit);
