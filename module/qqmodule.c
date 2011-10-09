#include <linux/module.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define MAX_LIST_SIZE 30
#define CONN_MAP_LIST_SIZE 400
#define LF_LEAVE 1
#define LF_DESTROY 2
#define LF_SEND 1
#define LF_RECEIVE 2

wait_queue_head_t wait_queue __initdata;

typedef struct {
    char * name;
    int pid_number;
    int tail;
    int head;
    char * messages[MAX_LIST_SIZE];
} Queue;

Queue * queue_list[MAX_LIST_SIZE] __initdata;

typedef struct {
	pid_t pid;
	int queue_id;
} Connection;

Connection * map[CONN_MAP_LIST_SIZE] __initdata;

int free_map_slots __initdata = CONN_MAP_LIST_SIZE; 

static int __init qqmodule_init(void) {
	DECLARE_WAIT_QUEUE_HEAD(wait_queue);
    printk("qqmodule init");
    return 0;
}

static void __exit qqmodule_exit(void) {
    printk("qqmodule exited");
    // TODO Free ALL THE THINGS
}

// old value e new value são endereços, dai inteiros.
int cas(int* cell, int oldvalue, int newvalue) {
	char result;

	asm ("lock cmpxchg %2, (%1) ; setz %0"
		: "=r" (result)
		: "r" (cell), "r" (newvalue), "a" (oldvalue));
	return result;
}

/* CAS-ENQUEUE
 * Sends a message to queue_id.
 */
int send_message(int queue_id, char* msg, int size) {
	int tail;
	char * x;
	char * cp_msg;
	Queue * q;
	
	cp_msg = kmalloc(sizeof(char)*size, GFP_KERNEL); // might be sizeof(char *)
	q = queue_list[queue_id];
	copy_from_user(cp_msg, msg, size);

	do {
		tail = q->tail;
		x = q->messages[tail % MAX_LIST_SIZE];
		if(tail != q->tail)
			continue;
		if(tail == q->head + MAX_LIST_SIZE) {
			wait_event(wait_queue, (tail != q->head + MAX_LIST_SIZE));
			continue;
		}
		if(x == NULL) {
			q->messages[tail % MAX_LIST_SIZE] = kmalloc(sizeof(char) * size, GFP_KERNEL);
			if(cas((int *) q->messages[tail % MAX_LIST_SIZE], (int) NULL, (int) cp_msg)) {
				cas(&q->tail, tail, tail+1);
				wake_up(&wait_queue);
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
int receive_message(int queue_id, char* msg, int size) {
	int head;
	char * x;
	Queue * q;
	
	q = queue_list[queue_id];

	do {
		head = q->head;
		x = q->messages[head % MAX_LIST_SIZE];
		if(head != q->head)
			continue;
		if(head == q->tail) {
			wait_event(wait_queue, (q->head != q->tail));
			continue;
		}
		if(x != NULL) {
			if(cas((int *) q->messages[head % MAX_LIST_SIZE], (int) x, (int) NULL)) {
				cas(&q->head, head, head+1);
				copy_to_user(msg, x, size);
				kfree(x);
				wake_up(&wait_queue);
				break;
			}
		} else {
			cas(&q->head, head, head+1);
		}
	} while(true);

	return 0;
}

int sys_qqmodule(int op, int queue_id, char* msg, int size) {
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
Queue * new_queue(char* name, pid_t pid) {
	Queue * q = kmalloc(sizeof(Queue), GFP_KERNEL);
	q->name = name;
	q->pid_number = 1;
	q->tail=0;
	q->head=0;

	return q;
}

/*
 * Creates a queue with the given name at the first available position.
 * If the operation is successful returns the position. If not
 * returns a negative value.
 * Pre-condition: There is no previously created queue with this name.
 */
int create_queue(char* name, pid_t pid) {
	Queue * q;
	int i;

	q = new_queue(name, pid);

	for(i = 0; i < MAX_LIST_SIZE; i++) {
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
int get_queue(char * name, pid_t pid) {
	Queue * current_queue;
	int i;

	for(i = 0; i < MAX_LIST_SIZE; i++) {
		current_queue = queue_list[i];
		// Queue exists, return it
		if(strcmp(name, current_queue->name))
			return i;
	}
	// Queue does not exist, create it
	return create_queue(name, pid);
}

Connection * create_connection(pid_t pid, int queue_id) {
	Connection * c = kmalloc(sizeof(Connection), GFP_KERNEL);
	c->pid = pid;
	c->queue_id = queue_id;

	return c;
}

/* Attachs the given pid to the queue with the given queue_id. */
int queue_attach(pid_t pid, int queue_id) {
	Connection * c;
	Queue * q;
	int i;
	int hasChanged = 0;

	c = create_connection(pid, queue_id);
	q = queue_list[queue_id];

	for(i = 0; i < MAX_LIST_SIZE; i++) {
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
int sys_qqmodule_named_attach(char* name, pid_t pid) {
	char* kname;
	int name_length;
	int queue_id;

	if(name == NULL)
		return -1;

	name_length = strlen(name);
	kname = kmalloc(sizeof(char) * name_length, GFP_KERNEL);
	// Returns 0 (false) if it was successfull (yeah, it confuses me too)
	if(copy_from_user(kname, name, name_length) != 0)
		return -1;

	if(free_map_slots == 0)
		return -2;

	queue_id = get_queue(kname, pid);
	queue_attach(pid, queue_id);
    return queue_id;
}

/*
 * Destroys the queue with the given queue_id, unlocking
 * all locked processes.
 * TODO Unlock the locked processes
 */
int destroy_queue(int queue_id) {
	int i;
	Queue * q;

	q = queue_list[queue_id];
//	Something like this for the process unlocking (+thread-safety)
//	for(int i = 0; i < q.pid_tail; i++)
//		unlock(q.pid_list[i])

	if(cas((int *) &queue_list[queue_id], (int) &queue_list[queue_id], (int) NULL)) {
		kfree(&q);
		for(i = 0; i < MAX_LIST_SIZE; i++) {
			if(map[i]->queue_id == queue_id) {
				if(cas((int *) map[i], (int) map[i], (int) NULL))
					cas(&free_map_slots, free_map_slots, free_map_slots+1);
			}
		}
	} else
		return -1;
	return 0;
}

/* Removes the given pid from the queue with the given queue_id */
int leave_queue(int queue_id, int pid) {
	int conn_index;
	int hasFound;
	
	hasFound = 0;

	if(queue_id >= MAX_LIST_SIZE)
		return -1;

	for(conn_index = 0; conn_index < CONN_MAP_LIST_SIZE; conn_index++) {
		if(map[conn_index] != NULL &&
		   map[conn_index]->pid == pid &&
		   map[conn_index]->queue_id == queue_id) {
		   	hasFound=1;
		   	if(cas((int *) map[conn_index], (int) map[conn_index], (int) NULL))
		   		cas(&free_map_slots, free_map_slots, free_map_slots+1);
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

int sys_qqmodule_named(int op, int queue_id, pid_t pid) {
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
