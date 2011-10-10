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

static Connection * map[MAP_SIZE];

static int free_map_slots __initdata = MAP_SIZE; 

static int destroy_queue(int queue_id);

static int __init qqmodule_init(void) {
	int i; 

	for(i = 0; i < LIST_SIZE; i++)
		queue_list[i] = NULL;
	for(i = 0; i < MAP_SIZE; i++)
		map[i] = NULL;

    printk("qqmodule init\n");
    return 0;
}

static void __exit qqmodule_exit(void) {
	int i;

	for(i = 0; i < LIST_SIZE; i++) {
		if(queue_list[i] != NULL) {
			printk("qqmodule Destroying queue %d\n", i);
			destroy_queue(i);
		}
	}

    printk("qqmodule exited\n");
}

// old value e new value são endereços, dai inteiros.
static int cas(int* cell, int oldvalue, int newvalue) {
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
	char * x;
	char * cp_msg;
	Queue * q;
	
	cp_msg = kmalloc(sizeof(char) * size, GFP_KERNEL); // might be sizeof(char *)
	q = queue_list[queue_id];
	copy_from_user(cp_msg, msg, size);

	do {
		tail = q->tail;
		x = q->messages[tail % LIST_SIZE];
		if(tail != q->tail)
			continue;
		if(tail == q->head + LIST_SIZE) {
			wait_event(q->wait_queue, (tail != q->head + LIST_SIZE));
			continue;
		}
		if(x == NULL) {
			q->messages[tail % LIST_SIZE] = kmalloc(sizeof(char) * size, GFP_KERNEL);
			if(cas((int *) q->messages[tail % LIST_SIZE], (int) NULL, (int) cp_msg)) {
				cas(&q->messages_size[tail % LIST_SIZE], 0, size);
				cas(&q->tail, tail, tail+1);
				wake_up(&(q->wait_queue));
				break;
			}
		} else {
			cas(&q->tail, tail, tail+1);
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
	Queue * q;
	int size_to_retrieve;	
	
	q = queue_list[queue_id];

	do {
		head = q->head;
		x = q->messages[head % LIST_SIZE];
		if(head != q->head)
			continue;
		if(head == q->tail) {
			wait_event(q->wait_queue, (q->head != q->tail));
			continue;
		}
		if(x != NULL) {
			if(cas((int *) q->messages[head % LIST_SIZE], (int) x, (int) NULL)) {
				cas(&q->head, head, head+1);

				if(size > q->messages_size[head % LIST_SIZE])
					size_to_retrieve = q->messages_size[head % LIST_SIZE];
				else
					size_to_retrieve = size;

				copy_to_user(msg, x, size_to_retrieve);
				kfree(x);

				cas(&q->messages_size[head % LIST_SIZE], q->messages_size[head % LIST_SIZE], 0);
				wake_up(&(q->wait_queue));
				break;
			}
		} else {
			cas(&q->head, head, head+1);
		}
	} while(true);

	return 0;
}

static int sys_qqmodule(int op, int queue_id, char* msg, int size) {
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
	Queue * q = kmalloc(sizeof(Queue), GFP_KERNEL);
	q->name = name;
	q->pid_number = 1;
	q->tail = 0;
	q->head = 0;
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

	q = new_queue(name, pid);

	for(i = 0; i < LIST_SIZE; i++) {
		if(queue_list[i] == NULL) {
			if(cas((int *) queue_list[i], (int) NULL, (int) &q))
				return i;
			else
				continue;
		}
	}
	return -1;
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
		current_queue = queue_list[i];
		// Queue exists, return it
		if(strcmp(name, current_queue->name))
			return i;
	}
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
	int hasChanged = 0;

	c = create_connection(pid, queue_id);
	q = queue_list[queue_id];

	for(i = 0; i < LIST_SIZE; i++) {
		if(map[i] == NULL) {
			if(cas((int *) &map[i], (int) NULL, (int) &c)) {
				cas(&q->pid_number, q->pid_number, q->pid_number+1);
				cas(&free_map_slots, free_map_slots, free_map_slots-1);
				hasChanged = 1;
			} else
				continue; // Caso não consiga trocar o map[i] continua o loop até encontrar outro null.
		}
	}
	if(hasChanged == 0)
		return -1; // Failed.
	return 0;
}

/*
 * Attach a given process to the queue with the given name.
 * If a queue with the given name does not exist, creates it.
 * If everything goes well, returns a non-negative int representing
 * the queue_id, else returns a negative int.
 */
static int sys_qqmodule_named_attach(char * name, pid_t pid) {
	char * kname;
	int name_length;
	int queue_id;
	int i;

	if(name == NULL)
		return -1; // Invalid Name

	name_length = strlen(name);
	kname = kmalloc(sizeof(char) * name_length, GFP_KERNEL);
	// Returns 0 (false) if it was successfull (yeah, it confuses me too)
	if(copy_from_user(kname, name, name_length) != 0)
		return -1;

	if(free_map_slots == 0)
		return -2; // No free map slots
	
	for(i = 0; i < MAP_SIZE; i++) {
		if(map[i] != NULL &&
		   map[i]->pid == pid &&
		   map[i]->queue_id == queue_id) {
		   return -3; // Already attached
		}
	}

	queue_id = get_queue(kname, pid);
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
	Queue * q;
	Connection * c;

	down(&lfdestroy_mutex);
	q = queue_list[queue_id];
//	Something like this for the process unlocking (+thread-safety)
//	for(int i = 0; i < q.pid_tail; i++)
//		unlock(q.pid_list[i])

	// In case another process destroyed it first;
	if(q == NULL)
		return -2;

	if(cas((int *) queue_list[queue_id], (int) queue_list[queue_id], (int) NULL)) {
		kfree(q);
		for(i = 0; i < LIST_SIZE; i++) {
			if(map[i]->queue_id == queue_id) {
				c = map[i];
				if(cas((int *) map[i], (int) map[i], (int) NULL)) {
					cas(&free_map_slots, free_map_slots, free_map_slots+1);
					kfree(c);
				}
			}
		}
	} else {
		up(&lfdestroy_mutex);
		return -1;
	}

	up(&lfdestroy_mutex);
	return 0;
}

/* Removes the given pid from the queue with the given queue_id */
static int leave_queue(int queue_id, int pid) {
	int conn_index;
	int hasFound;
	Connection * c;
	
	hasFound = 0;

	if(queue_id >= LIST_SIZE)
		return -1;

	for(conn_index = 0; conn_index < MAP_SIZE; conn_index++) {
		if(map[conn_index] != NULL &&
		   map[conn_index]->pid == pid &&
		   map[conn_index]->queue_id == queue_id) {
		   	hasFound=1;
		   	c = map[conn_index];
		   	if(cas((int *) map[conn_index], (int) map[conn_index], (int) NULL)) {
		   		cas(&free_map_slots, free_map_slots, free_map_slots+1);
		   		kfree(c);
		   	}
			break;
		}
	}

	// Not found
	if(hasFound == 0)
		return -2;

	if(queue_list[queue_id]->pid_number == 1) // Vai ter de ser mudado provavelmente
		destroy_queue(queue_id);
	else
		queue_list[queue_id]->pid_number -= 1;

	return 0;
}

static int sys_qqmodule_named(int op, int queue_id, pid_t pid) {
	switch(op) {
	case LF_LEAVE:
		return leave_queue(queue_id, pid);
	case LF_DESTROY:
		return destroy_queue(queue_id);
	}
	return -5; // Unsupported OP
}


module_init(qqmodule_init);
module_exit(qqmodule_exit);
