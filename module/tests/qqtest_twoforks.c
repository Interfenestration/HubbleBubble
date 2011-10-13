#include <unistd.h>
#include <sys/qqservice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int for_me(char * msg, char me) {
	return msg[3] == me;
}

/* Cria tres processos que comunicam entre si utilizando uma so queue */
int forkItTwice() {
	int queue_id;
	int success;
	char * msgs[9];
	char * received = calloc(sizeof(char), 50);
	char * queue_name = "family";
	int messages_wanted = 3;
	int messages_received = 0;
	msgs[0] = "fc2c"; // From Child To Child
	msgs[1] = "fc2p"; // From Child To Parent
	msgs[2] = "fc2g"; // From Child To Grandparent
	msgs[3] = "fp2c"; // ...
	msgs[4] = "fp2p";
	msgs[5] = "fp2g";
	msgs[6] = "fg2c";
	msgs[7] = "fg2p";
	msgs[8] = "fg2g"; // From Grandparent To Grandparent

	printf("Fork It Twice Test Started\n");
	success = fork();
	if(success == -1) {
		printf("Fork Failed\n");
	} else if(success == 0) { // Parent
		success = fork();
		if(success == 0) { // Child
			queue_id = lfattach(queue_name, strlen(queue_name));
			lfsend(queue_id, msgs[0], strlen(msgs[0]));
			lfsend(queue_id, msgs[1], strlen(msgs[1]));
			lfsend(queue_id, msgs[2], strlen(msgs[2]));
			while(messages_received < messages_wanted) {
				lfreceive(queue_id, received, 50);
				if(for_me(received, 'c')) {
					printf("Child received: %s\n", received);
					messages_received++;
				}
				else {
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
			exit(0);
		}
		else { // Still Parent
			queue_id = lfattach(queue_name, strlen(queue_name));
			lfsend(queue_id, msgs[3], strlen(msgs[3]));
			lfsend(queue_id, msgs[4], strlen(msgs[4]));
			lfsend(queue_id, msgs[5], strlen(msgs[5]));
			while(messages_received < messages_wanted) {
				lfreceive(queue_id, received, 50);
				if(for_me(received, 'p')) {
					printf("Parent received: %s\n", received);
					messages_received++;
				}
				else {
		    		lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
			exit(0);
		}
	}
	else { // Grandparent
		queue_id = lfattach(queue_name, strlen(queue_name));
		lfsend(queue_id, msgs[6], strlen(msgs[6]));
		lfsend(queue_id, msgs[7], strlen(msgs[7]));
		lfsend(queue_id, msgs[8], strlen(msgs[8]));
		while(messages_received < messages_wanted) {
			lfreceive(queue_id, received, 50);
			if(for_me(received, 'g')) {
				printf("Grandparent received: %s\n", received);
				messages_received++;
			}
			else {
				//printf("Grandparent says: Not for me... %s\n", received);
				lfsend(queue_id, received, 50);
			}
		}
		lfleave(queue_id);
		exit(0);
	}

	printf("Fork It Twice Test Ended\n\n");
	return 0;
}

int main(int argc, char * argv[]) {
    return forkItTwice();
}
