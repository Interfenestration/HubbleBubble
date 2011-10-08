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

wait_queue_head_t wait_queue;

struct Queue {
    char * name;
    int pid_number;
    int messages_tail;
    int messages_head;
    char * messages[MAX_LIST_SIZE];
} queue_list[MAX_LIST_SIZE] __init_data;

struct Connection {
	pid_t pid;
	int queue_id;
} map[CONN_MAP_LIST_SIZE] __init_data;

int free_map_slots __init_data = CONN_MAP_LIST_SIZE; 

static int __init qqmodule_init(void) {
	DECLARE_WAIT_QUEUE_HEAD(wait_queue);
    printk("qqmodule init");
    return 0;
}

static void __exit qqmodule_exit(void) {
    printk("qqmodule exited");
    // TODO Free ALL THE THINGS
}

int sys_qqmodule(int op, int queue_id, char* msg, int size) {
    switch(op) {
	case LF_SEND:
		return send_message(queue_id, msg, size);
	case LF_RECEIVE:
		return receive_message(queue_id, msg, size);
	}
}

/* CAS-ENQUEUE
 * Sends a message to queue_id.
 */
int send_message(int queue_id, char* msg, int size) {
	int tail, x;
	char * cp_msg = kmalloc(sizeof(char)*size, GFP_KERNEL); // might be sizeof(char *)
	copy_from_user(cp_msg, msg, size);
	Queue q = queue_list[queue_id];

	do {
		tail = q.messages_tail;
		x = q.messages[tail % MAX_LIST_SIZE];
		if(tail != q.messages_tail)
			continue;
		if(tail == q.messages_head + MAX_LIST_SIZE) {
			wait_event(&wait_queue, (tail != q.messages_head + MAX_LIST_SIZE));
			continue;
		}
		if(x == NULL) {
			q.messages[tail % MAX_LIST_SIZE] = kmalloc(sizeof(char)*size, GFP_KERNEL);
			if(cas(&q.messages[tail % MAX_LIST_SIZE], NULL, (int) cp_msg)) {
				cas(&q.messages_tail, tail, tail+1);
				wake_up(&wait_queue);
				break;
			}
		} else {
			cas(&q.messages_tail, tail, tail+1);
		}
	} while(true)

	return 0;
}

/* CAS-DEQUEUE
 * Retrieves a message from queue_id.
 * Pre-Condition - msg must be malloc'd (user side).
 */
int receive_message(int queue_id, char* msg, int size) {
	int head, x;
	Queue q = queue_list[queue_id];

	do {
		head = q.messages_head;
		x = q.messages[head % MAX_LIST_SIZE];
		if(head != q.messages_head)
			continue;
		if(head == q.messages_tail) {
			wait_event(&wait_queue, (q.messages_head != q.messages_tail));
			continue;
		}
		if(x != NULL) {
			if(cas(&q.messages[head % MAX_LIST_SIZE], x, (int) NULL)) {
				cas(&q.messages_head, head, head+1);
				copy_to_user(msg, x, size);
				kfree(x);
				wake_up(&wait_queue);
				break;
			}
		} else {
			cas(&q.messages_head, head, head+1);
		}
	} while(true)
	return 0;
}

// old value e new value são endereços, dai inteiros.
int cas(int* cell, int oldvalue, int newvalue) {
	char result;
	asm ("lock cmpxchg %2, (%1) ; setz %0"
		: "=r" (result)
		: "r" (cell), "r" (newvalue), "a" (oldvalue));
	return result;
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

/* Attachs the given pid to the queue with the given queue_id. */
int queue_attach(pid_t pid, int queue_id) {
	Connection c;
	Queue q;
	int i;
	int hasChanged = 0;
	c = create_connection(pid, queue_id);
	q = queue_list[queue_id];

	for(i = 0; i < MAX_LIST_SIZE; i++) {
		if(map[i] == NULL) {
			if(cas(&map[i], NULL, c)) {
				cas(&q.pid_number, q.pid_number, q.pid_number+1);
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

Connection create_connection(pid_t pid, int queue_id) {
	Connection c = {
		.pid = pid;
		.queue_id = queue_id;
	}
	return c;
}

/*
 * Tries to find a queue with the given name.
 * If it exists, it simply returns its index. If not, it creates
 * a new one, adds it to the array and returns its index.
 */
int get_queue(char* name, pid_t pid) {
	Queue current_queue;
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
 */
int create_queue(char* name, pid_t pid) {
	Queue q;
	int i;

	q = new_queue(char* name, pid);

	for(i = 0; i < MAX_LIST_SIZE; i++) {
		if(queue_list[i] == NULL) {
			if(cas(&queue_list[i], NULL, q))
				return i;
			else
				continue;
		}
	}
	return -1;
}

/*
 * Creates, inits and returns a queue struct with the given name.
 */
Queue new_queue(char* name, pid_t pid) {
	Queue q = {
		.name = name;
		.pid_list[0] = pid;
		.pid_number = 1;
		.messages_tail = 0;
		.messages_head = 0;
		.messages = kmalloc(sizeof(char*)*MAX_LIST_SIZE, GFP_KERNEL);
	}
	return q;
}

int sys_qqmodule_named(int op, int queue_id, pid_t pid) {
	switch(op) {
	case LF_LEAVE:
		return leave_queue(queue_id, pid);
	case LF_DESTROY:
		return destroy_queue(queue_id);
	}
}

/* Removes the given pid from the queue with the given queue_id */
int leave_queue(int queue_id, int pid) {
	int conn_index;
	int hadFound = 0;

	if(queue_id >= MAX_LIST_SIZE)
		return -1;

	for(conn_index = 0; i < CONN_MAP_LIST_SIZE; i++)
		if(map[conn_index] != NULL &&
		   map[conn_index].pid == pid &&
		   map[conn_index].queue_id == queue_id) {
		   	hasFound=1;
			break;
		}

	// Not found
	if(hasFound == 0)
		return -2;

	cas(&map[conn_index], map[conn_index], NULL);
	cas(&free_map_slots, free_map_slots, free_map_slots+1);

	if(queue_list[queue_id].pid_number == 1)
		destroy_queue(queue_id);
	else
		queue_list[queue_id].pid_number -= 1;

	return 0;
}

/*
 * Destroys the queue with the given queue_id, unlocking
 * all locked processes.
 * TODO Unlock the locked processes
 */
int destroy_queue(int queue_id) {
	int i;
	Queue q;

	q = queue_list[queue_id];
//	Something like this for the process unlocking (+thread-safety)
//	for(int i = 0; i < q.pid_tail; i++)
//		unlock(q.pid_list[i])
	
	if(cas(&queue_list[queue_id], queue_list[queue_id], NULL)) {
		kfree(&q);
		for(i = 0; i < MAX_LIST_SIZE; i++) {
			if(map[i].queue_id == queue_id) {
				if(cas(&map[i], map[i], NULL))
					cas(&free_map_slots, free_map_slots, free_map_slots+1);
			}
		}
	} else
		return -1;
	return 0;
}


module_init(qqmodule_init);
module_exit(qqmodule_exit);
