#include <linux/module.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
MODULE_LICENSE("GPL");

#define LIST_SIZE 30
#define MAP_SIZE 400
#define LF_LEAVE 1
#define LF_DESTROY 2
#define LF_SEND 1
#define LF_RECEIVE 2

static DECLARE_MUTEX(lfdestroy_mutex);

typedef struct {
    char * name;
    int pid_number;
    int tail;
    int head;
    int messages_size[LIST_SIZE];
    char * messages[LIST_SIZE];
    wait_queue_head_t wait_queue;
} Queue;

static Queue * queue_list[LIST_SIZE];

typedef struct {
	pid_t pid;
	int queue_id;
} Connection;

extern int (* sys_qqmodule) (int, int, char *, int);
extern int (* sys_qqmodule_named_attach) (char *, int, pid_t);
extern int (* sys_qqmodule_named) (int, int, pid_t);

static Connection * map[MAP_SIZE];

static int free_map_slots = MAP_SIZE; 

static int destroy_queue(int queue_id);

// old value e new value são endereços, dai inteiros.
static int cas(void * cell, int oldvalue, int newvalue) {
	char result;

	asm ("lock cmpxchg %2, (%1) ; setz %0"
		: "=r" (result)
		: "r" (cell), "r" (newvalue), "a" (oldvalue));
	return result;
}

/* CAS-ENQUEUE
 * Sends a message to queue_id.
 */
static int send_message(int queue_id, char * msg, int size) {
	int tail;
	int res;
	char * x;
	char * cp_msg;
	Queue * q;
	
	printk("qqmodule: send_message start\n");

	cp_msg = kmalloc(sizeof(char) * size, GFP_KERNEL); // might be sizeof(char *)

	printk("qqmodule: send_message trying to access queue %d; size: %d", queue_id, size);

	q = queue_list[queue_id];

	if(q == 0)
		return -3; // queue not found.
	
	printk("qqmodule: send_message -> copy_from_user\n");
	copy_from_user(cp_msg, msg, size); // Check the size. might be bigger than its suposed to.

	printk("qqmodule: send_message -> looooop\n");

	do {
		tail = q->tail;
		x = q->messages[tail % LIST_SIZE];
		if(tail != q->tail) {
			printk("qqmodule: send_message -> tail != q->tail\n");
			continue;
		}
		if(tail == q->head + LIST_SIZE) {
			printk("qqmodule: send_message -> going to wait...\n");
			wait_event(q->wait_queue, (tail != q->head + LIST_SIZE));
			continue;
		}
		if(x == 0) {
			printk("qqmodule: send_message -> before CAS\n");
			if(cas(&(q->messages[tail % LIST_SIZE]), 0, (int) cp_msg)) {
				printk("qqmodule: send_message -> msg = %s\n", q->messages[tail % LIST_SIZE]);

				res = cas(&(q->tail), tail, tail+1);
				res = cas(&(q->messages_size[tail % LIST_SIZE]), 0, size);

				wake_up(&(q->wait_queue));
				break;
			}
		} else {
			printk("qqmodule: send_message -> X!=0 \n");

			res = cas(&(q->tail), tail, tail+1);
		}
	} while(true);

	return 0;


}

/* CAS-DEQUEUE
 * Retrieves a message from queue_id.
 * Pre-Condition - msg must be malloc'd (user side).
 */
static int receive_message(int queue_id, char * msg, int size) {
	int head;
	char * x;
	int res;
	Queue * q;
	int size_to_retrieve = -1;	

	printk("qqmodule: receive_message start\n");

	
	q = queue_list[queue_id];

	printk("qqmodule: receive_message -> looooooooooop\n");

	do {
		head = q->head;
		x = q->messages[head % LIST_SIZE];
		if(head != q->head) {
			printk("qqmodule: receive_message -> head != q->head\n");
			continue;
		}
		if(head == q->tail) {
			printk("qqmodule: receive_message -> going to wait...\n");
			wait_event(q->wait_queue, (q->head != q->tail));
			continue;
		}
		if(x != 0) {
			printk("qqmodule: receive_message -> before CAS\n");
			if(cas(&(q->messages[head % LIST_SIZE]), (int) x, 0)) {
				res = cas(&(q->head), head, head+1);

				if(size > q->messages_size[head % LIST_SIZE])
					size_to_retrieve = q->messages_size[head % LIST_SIZE];
				else
					size_to_retrieve = size;

				copy_to_user(msg, x, size_to_retrieve);
				kfree(x);

				res = cas(&(q->messages_size[head % LIST_SIZE]), q->messages_size[head % LIST_SIZE], 0);
				wake_up(&(q->wait_queue));
				break;
			}
		} else {
			res = cas(&(q->head), head, head+1);
		}
		printk("qqmodule: receive_message -> trying again to receive\n");
	} while(true);

	return size_to_retrieve;
}

int sys_qqmodule_impl(int op, int queue_id, char* msg, int size) {
    switch(op) {
	case LF_SEND:
		return send_message(queue_id, msg, size);
	case LF_RECEIVE:
		return receive_message(queue_id, msg, size);
	}
	return -5; // Unsupported OP
}

/*
 * Creates, inits and returns a queue struct with the given name.
 */
static Queue * new_queue(char * name, pid_t pid) {
	int i;
	Queue * q = kmalloc(sizeof(Queue), GFP_KERNEL);
	q->name = name;
	q->pid_number = 1;
	q->tail = 0;
	q->head = 0;
	
	for(i = 0; i < LIST_SIZE; i++) {
		q->messages[i] = 0;
		q->messages_size[i] = 0;
	}

	printk("qqmodule: new_queueue -> entering init_waitqueueue\n");
	init_waitqueue_head(&(q->wait_queue));

	return q;
}

/*
 * Creates a queue with the given name at the first available position.
 * If the operation is successful returns the position. If not
 * returns a negative value.
 * Pre-condition: There is no previously created queue with this name.
 */
static int create_queue(char * name, pid_t pid) {
	Queue * q;
	int i;

	printk("qqmodule: create_queueue -> entering new_queue\n");
	q = new_queue(name, pid);

	printk("qqmodule: create_queueue -> entering loop\n");
	for(i = 0; i < LIST_SIZE; i++) {
		if(cas(&(queue_list[i]), 0, (int) q)) {
			printk("qqmodule: create_queueue -> returning %d\n", i);
			return i;
		}
		else
			continue;
	}
	return -2;
}

/*
 * Tries to find a queue with the given name.
 * If it exists, it simply returns its index. If not, it creates
 * a new one, adds it to the array and returns its index.
 */
static int get_queue(char * name, pid_t pid) {
	Queue * current_queue;
	int i;

	for(i = 0; i < LIST_SIZE; i++) {
		if(queue_list[i] != NULL) {
			current_queue = queue_list[i];
			// Queue exists, return it
			if(strcmp(name, current_queue->name) == 0)
				return i;
		}
	}
	printk("qqmodule: get_queueue -> returning create_queue\n");
	// Queue does not exist, create it
	return create_queue(name, pid);
}

static Connection * create_connection(pid_t pid, int queue_id) {
	Connection * c = kmalloc(sizeof(Connection), GFP_KERNEL);
	c->pid = pid;
	c->queue_id = queue_id;

	return c;
}

/* Attachs the given pid to the queue with the given queue_id. */
static int queue_attach(pid_t pid, int queue_id) {
	Connection * c;
	Queue * q;
	int i;
	int res;
	int hasChanged = 0;

	c = create_connection(pid, queue_id);
	q = queue_list[queue_id];

	printk("qqmodule: queue_attach -> entering looooooooop\n");


	for(i = 0; i < LIST_SIZE; i++) {
		if(cas(&(map[i]), 0, (int) c)) {
			hasChanged = 1;
			printk("qqmodule: queue_attach -> occupying %d\n", i);

			res = cas(&free_map_slots, free_map_slots, free_map_slots-1);
			printk("qqmodule: queue_attach -> passed free_map_slots\n");

			break;
		} else
			continue; // Caso não consiga trocar o map[i] continua o loop até encontrar outro null.
	}

	printk("qqmodule: queue_attach -> out of loooooooooop\n");

	if(hasChanged == 0)
		return -2; // Failed.

	return 0;
}

/*
 * Attach a given process to the queue with the given name.
 * If a queue with the given name does not exist, creates it.
 * If everything goes well, returns a non-negative int representing
 * the queue_id, else returns a negative int.
 */
int sys_qqmodule_named_attach_impl(char * name, int size, pid_t pid) {
	char * kname;
	int queue_id;
	int i;

	kname = kmalloc(sizeof(char) * size, GFP_KERNEL);
	// Returns 0 (false) if it was successfull (yeah, it confuses me too)
	if(copy_from_user(kname, name, size) != 0)
		return -2;

	printk("qqmodule: named_attach_impl -> copied from user\n");

	if(free_map_slots == 0)
		return -3; // No free map slots

	queue_id = get_queue(kname, pid);

	printk("qqmodule: named_attach_impl -> entering looooooooop\n");


	for(i = 0; i < MAP_SIZE; i++) {
		if(map[i] != NULL) 
			if(map[i]->pid == pid && map[i]->queue_id == queue_id) {
		   		return -4; // Already attached
			}
	}

	printk("qqmodule: named_attach_impl -> entering queueueue_attach\n");

	queue_attach(pid, queue_id);
    return queue_id;
}

/*
 * Destroys the queue with the given queue_id, unlocking
 * all locked processes.
 * TODO Unlock the locked processes
 */
static int destroy_queue(int queue_id) {
	int i;
	int res;
	Queue * q;
	Connection * c;

	printk("qqmodule: destroy_queueue -> locking\n");
	down(&lfdestroy_mutex);
	q = queue_list[queue_id];

	// UNLOCK PROCESSES
	// In case another process destroyed it first;
	if(q == 0)
		return -2;
	
	if(cas(&(queue_list[queue_id]), (int) queue_list[queue_id], 0)) {
		kfree(q);
		printk("qqmodule: destroy_queueue -> loooooooop\n");
		for(i = 0; i < LIST_SIZE; i++) {
			if(map[i] != 0) {
				if(map[i]->queue_id == queue_id) {
					c = map[i];
					if(cas(&(map[i]), (int) map[i], 0)) {
						res = cas(&free_map_slots, free_map_slots, free_map_slots+1);
						kfree(c);
					}
				}
			}
		}
	} else {
		up(&lfdestroy_mutex);
		return -3;
	}

	up(&lfdestroy_mutex);
	return 0;
}

/* Removes the given pid from the queue with the given queue_id */
static int leave_queue(int queue_id, int pid) {
	int conn_index;
	int hasFound;
	int res;
	Connection * c;
	
	hasFound = 0;

	if(queue_id >= LIST_SIZE)
		return -2;

	for(conn_index = 0; conn_index < MAP_SIZE; conn_index++) {
		if(map[conn_index] != NULL &&
		   map[conn_index]->pid == pid &&
		   map[conn_index]->queue_id == queue_id) {
		   	hasFound=1;
		   	c = map[conn_index];
		   	if(cas(&(map[conn_index]), (int) map[conn_index], 0)) {
		   		res = cas(&free_map_slots, free_map_slots, free_map_slots+1);
		   		kfree(c);
		   	}
			break;
		}
	}

	// Not found
	if(hasFound == 0)
		return -3;

	if(queue_list[queue_id]->pid_number == 1) // Vai ter de ser mudado provavelmente
		destroy_queue(queue_id);
	else
		queue_list[queue_id]->pid_number -= 1;

	return 0;
}

int sys_qqmodule_named_impl(int op, int queue_id, pid_t pid) {
	switch(op) {
	case LF_LEAVE:
		return leave_queue(queue_id, pid);
	case LF_DESTROY:
		return destroy_queue(queue_id);
	}
	return -5; // Unsupported OP
}

static int __init qqmodule_init(void) {
	int i;

	sys_qqmodule = &sys_qqmodule_impl;
	sys_qqmodule_named = &sys_qqmodule_named_impl;
	sys_qqmodule_named_attach = &sys_qqmodule_named_attach_impl;
	
	for(i = 0; i < LIST_SIZE; i++)
		queue_list[i] = 0;
	for(i = 0; i < MAP_SIZE; i++)
		map[i] = 0;

    printk("qqmodule init\n");
    return 0;
}

static void __exit qqmodule_exit(void) {
	int i;

	sys_qqmodule = 0;
	sys_qqmodule_named = 0;
	sys_qqmodule_named_attach = 0;

	for(i = 0; i < LIST_SIZE; i++) {
		if(queue_list[i] != NULL) {
			printk("qqmodule Destroying queue %d\n", i);
			destroy_queue(i);
		}
	}

    printk("qqmodule exited\n");
}

module_init(qqmodule_init);
module_exit(qqmodule_exit);
