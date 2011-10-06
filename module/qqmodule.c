#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define __NR_qqservice 325
#define __NR_qqservice_named 326
#define __NR_qqservice_named_attach 327

#define MAX_LIST_SIZE 30

struct queue {
    char * name;
    char * messages[MAX_LIST_SIZE];
} queue_list[MAX_LIST_SIZE]

static int __init qqmodule_init(void) {
    printk("qqmodule init");
    return 0;
}

static void __exit qqmodule_exit(void) {
    printk("qqmodule exited");
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

int lfsend(const void * msg, int size) {
    return syscall(__NR_qqservice, 1, cp_msg, size);
}

int lfreceive(void * msg, int size) {
    return syscall(__NR_qqservice, 2, cp_msg_size);
}

// Returns the id of the queue
int lfattach(const void *name) {
    return syscall(__NR_qqservice_named_attach, cp_name); // ID
}

int lfleave(int queueId) {
    return syscall(__NR_qqservice_named, 1, queueId);
}

int lfdestroy(int queueId) {
    return syscall(__NR_qqservice_named, 2, queueId);
}

module_init(qqmodule_init);
module_exit(qqmodule_exit);
