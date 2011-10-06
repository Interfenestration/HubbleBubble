#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/sched.h>

#define __NR_qqservice 325
#define __NR_qqservice_named 326
#define __NR_qqservice_named_attach 327

#define MAX_LIST_SIZE 30
#define LF_LEAVE 1
#define LF_DESTROY 2

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
 * the queue_id, else returns a negative number.
 */
int sys_qqmodule_named_attach(void * name, int pid) {
	if(name == NULL )
		return -1;
	// TODO And what happens if name is not char*?
	char * kname;
	int name_length;
	int queue_id;
	// TODO Is a cast to char* necessary?
	name_length = strlen_user(name);
	kname = malloc(sizeof(char) * name_length);
	// Returns 0 (false) if it was successfull (yeah, it confuses me too)
	if(copy_from_user(kname, name, name_length) != 0)
		return -1;
	
	queue_id = get_queue(kname, pid);
	queue_attach(pid, queue_id);
    return queue_id;
}

/* Attachs the given pid to the queue with the given queue_id. */
int queue_attach(int pid, int queue_id) {
	queue q = queue_list[queue_id];
	if(q.pid_tail <= MAX_LIST_SIZE)
		q.pid_list[tail++] = pid;
	// TODO else what? fails? block?
}

/*
 * Tries to find a queue with the given name.
 * If it exists, it simply returns its index. If not, it creates
 * a new one, adds it to the array and returns its index.
 */
int get_queue(char * name, int pid) {
	queue current_queue;
	int i;
	for(i = 0; i < MAX_LIST_SIZE; i++) {
		current_queue = queue_list[i];
		// Queue exists, return it
		if(strcmp(name, current_queue.name))
			return current_queue;
	}
	// Queue does not exist, create it
	return create_queue(name, pid);
}

/*
 * Creates a queue with the given name at the first available position.
 * If the operation is successful returns the position. If not
 * returns a negative value.
 * Pre-condition: There is no previously created queue with this name.
 * TODO Make this thread-safe
 */
int create_queue(char* name, int pid) {
	queue q;
	int i;
	q = new_queue(char* name, pid);

	for(i = 0; i < MAX_LIST_SIZE; i++) {
		if(queue_list[i] == NULL) {
			// TODO thread-safeness here
			queue_list[i] = q;
			return i;
		}
	}
	return -1;
}

/*
 * Creates, inits and returns a queue struct with the given name.
 */
queue new_queue(char* name, int pid) {
	queue q = {
		.name = name;
		.pid_list[0] = pid;
		.pid_tail = 1;
		.messages_tail = 0;
		.messages_head = 0;
	}
	return q;
}

int sys_qqmodule_named(int op, int queue_id, int pid) {
	switch(op) {
	case LF_LEAVE:
		return leave_queue(queue_id, pid);
	case LF_DESTROY:
		return destroy_queue(queue_id);
}

/* Removes the given pid from the queue with the given queue_id */
int leave_queue(int queue_id, int pid) {
	int[] pid_list;
	int pid_index, pid_tail;
	queue q;
	q = queue_list[queue_id];
	pid_list = q.pid_list;
	pid_tail = q.pid_tail;
	for(pid_index = 0; i < pid_tail; i++)
		if(pid_list[pid_index] == pid)
			break;
	
	if(pid_index >= pid_tail)
		return -1;

	// TODO insert thread-safeness
	pid_list[pid_index] = pid_list[pid_tail - 1];
	q.pid_tail--;
	return 0;
}
/* 
 * Destroys the queue with the given queue_id, unlocking
 * all locked processes.
 * TODO Unlock the locked processes
 */
int destroy_queue(int queue_id) {
	queue q = list_queue[queue_id];
//	Something like this for the process unlocking (+thread-safety)
//	for(int i = 0; i < q.pid_tail; i++)
//		unlock(q.pid_list[i])
	free(&queue_list[queue_id]);
	queue_list[queue_id] = NULL;
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
    return syscall(__NR_qqservice_named_attach, cp_name, getpid());
}

int lfleave(int queue_id) {
    return syscall(__NR_qqservice_named, LF_LEAVE, queue_id);
}

int lfdestroy(int queue_id) {
    return syscall(__NR_qqservice_named, LF_DESTROY, queue_id);
}


module_init(qqmodule_init);
module_exit(qqmodule_exit);
