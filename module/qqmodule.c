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
    int pid_list[MAX_LIST_SIZE];
    int pid_tail;
    int messages_tail;
    int messages_head;
    char * messages[MAX_LIST_SIZE];
} queue_list[MAX_LIST_SIZE] __init_data;

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

/*
 * Attached a given process to the queue with the given name.
 * If a queue with the given name does not exist, creates it.
 * If everything goes well, returns a non-negative int representing
 * the queueId, else returns a negative number.
 */
int sys_qqmodule_named_attach(void * name) {
	if(name == NULL)
		return -1;
	// TODO And what happens if name is not char*?
	char * kname;
	int name_length;
	// TODO Is a cast to char* necessary?
	name_length = strnlen(name);
	kname = malloc(sizeof(char) * name_length);
	// Returns 0 (false) if it was successfull (yeah, it confuses me too)
	if(copy_from_user(kname, name, name_length))
		return -1;
	
    return get_queue(kname);
}

/*
 * Tries to find a queue with the given name.
 * If it exists, it simply returns its index. If not, it creates
 * a new one, adds it to the array and returns its index.
 */
int get_queue(char * name) {
	queue current_queue;
	int i;
	for(i = 0; i < MAX_LIST_SIZE; i++) {
		current_queue = queue_list[i];
		// Queue exists, return it
		if(strcmp(name, current_queue.name))
			return current_queue;
	}
	// Queue does not exist, create it
	return create_queue(name);
}

/*
 * Creates a queue with the given name at the first available position.
 * If the operation is successful returns the position. If not
 * returns a negative value.
 * Pre-condition: There is no previously created queue with this name.
 * TODO Make this thread-safe
 * TODO What happens when there is no space?
 */
int create_queue(char* name) {
	queue q;
	int i;
	q = new_queue(char* name);
	for(i = 0; i < MAX_LIST_SIZE; i++) {
		if(queue_list[i] == NULL) {
			queue_list[i] = q;
			return i;
		}
	}
	// TODO No space available, now what? block or fail?
	return -1;
}

/*
 * Creates, inits and returns a queue struct with the given name.
 */
queue new_queue(char* name) {
	queue q = {
		.name = name;
		.pid_list[0] = getpid(); // TODO Is this really the pid or does the kernel mess it up?
		.pid_tail = 1;
		.messages_tail = 0;
		.messages_head = 0;
	}
	return q;
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

/*
 * Attaches the current process to the named queue.
 * Returns the id of the queue the process was attached to.
 */
int lfattach(const void *name) {
    return syscall(__NR_qqservice_named_attach, cp_name);
}

int lfleave(int queueId) {
    return syscall(__NR_qqservice_named, 1, queueId);
}

int lfdestroy(int queueId) {
    return syscall(__NR_qqservice_named, 2, queueId);
}

module_init(qqmodule_init);
module_exit(qqmodule_exit);
