#include <unistd.h>
#include <sys/qqservice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int forkItTwice();
int for_me(char[], char);
int forkItOnce();
int joinLeaveTest();
int pluralTest();
int boundsTest();
int simpleTest();
void println(char*);

int main(int argc, char * argv[]) {
	simpleTest();
	return 0;
}

int forkItTwice() {
	int queue_id;
	int success;
	char * msgs[9];
	char received[50];
	char queue_name[] = "family";
	int messages_wanted = 3;
	int messages_received = 0;
	msgs[0] = "fc2c-"; // From Child To Child
	msgs[1] = "fc2p-"; // From Child To Parent
	msgs[2] = "fc2g-"; // From Child To Grandparent
	msgs[3] = "fp2c-"; // ...
	msgs[4] = "fp2p-";
	msgs[5] = "fp2g-";
	msgs[6] = "fg2c-";
	msgs[7] = "fg2p-";
	msgs[8] = "fg2g-"; // From Grandparent To Grandparent

	printf("Fork It Twice Test Started\n");
	success = fork();
	if(success == -1) {
		printf("Fork Failed\n");
	}
	else if(success == 0) { // Parent
		success = fork();
		if(success == 0) { // Child
			queue_id = lfattach(queue_name);
			lfsend(queue_id, msgs[0], 50);
			lfsend(queue_id, msgs[1], 50);
			lfsend(queue_id, msgs[2], 50);
			while(messages_received < messages_wanted) {
				lfreceive(queue_id, received, 50);
				if(for_me(received, 'c')) {
					printf("Child received: %s\n", received);
					messages_received++;
				}
				else {
					printf("Child says: Not for me... %s\n", received);
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
		}
		else { // Still Parent
			queue_id = lfattach(queue_name);
			lfsend(queue_id, msgs[3], 50);
			lfsend(queue_id, msgs[4], 50);
			lfsend(queue_id, msgs[5], 50);
			while(messages_received < messages_wanted) {
				lfreceive(queue_id, received, 50);
				if(for_me(received, 'p')) {
					printf("Parent received: %s\n", received);
					messages_received++;
				}
				else {
					printf("Parent says: Not for me... %s\n", received);
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
		}
	}
	else { // Grandparent
		queue_id = lfattach(queue_name);
			lfsend(queue_id, msgs[6], 50);
			lfsend(queue_id, msgs[7], 50);
			lfsend(queue_id, msgs[8], 50);
			while(messages_received < messages_wanted) {
				lfreceive(queue_id, received, 50);
				if(for_me(received, 'g')) {
					printf("Grandparent received: %s\n", received);
					messages_received++;
				}
				else {
					printf("Grandparent says: Not for me... %s\n", received);
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
	}

	printf("Fork It Twice Test Ended\n\n");
	return 0;
}


int for_me(char * msg, char me) {
	return msg[3] == me;
}

int forkItOnce() {
	int childRead_parentWrite;
	int childWrite_parentRead;
	int success;
	char crpw[] = "crpw";
	char cwpr[] = "cwpr";
	char son1[] = "Hello, Parent";
	char father1[] = "Hello, Child";
	char son2[] = "Parent, where do Children come from?";
	char father21[] = "Well, you see Child...";
	char father22[] = "When a Process loves itself so much...";
	char father23[] = "It grabs a fork and...";
	char father24[] = "A Child is born!";
	char son3[] = "And how does the Parent know it's the Parent and the Child knows it's the Child?";
	char father31[] = "That's easy!";
	char father32[] = "Both check what the fork left behind...";
	char father33[] = "If what they see is nothing then they're the Child...";
	char father34[] = "If what the see is the Child then they're the Parent.";
	char son4[] = "Thank you, Parent.";
	char father4[] = "You're welcome, Child";
	char son5[] = "I hope you wait for me...";
	char father5[] = "I will...";
	char father6[] = "...Or not. You can be a zombie for all I care...";
	char receive[100];

	printf("Fork It Once Test Started\n");
	success = fork();
	if(success == -1) {
		printf("Fork Failed!");
	} else if(success == 0) { // Son
		childRead_parentWrite = lfattach(crpw);
		childWrite_parentRead = lfattach(cwpr);
		lfsend(childWrite_parentRead, son1, strlen(son1) + 1);
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father1
		lfsend(childWrite_parentRead, son2, strlen(son2) + 1);
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father21
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father22
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father23
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father24
		lfsend(childWrite_parentRead, son3, strlen(son3) + 1);
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father31
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father32
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father33
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father34
		lfsend(childWrite_parentRead, son4, strlen(son4) + 1);
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father4
		lfsend(childWrite_parentRead, son5, strlen(son5) + 1);
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s\n", receive); // father5
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
	} else { // Parent
		childRead_parentWrite = lfattach(crpw);
		childWrite_parentRead = lfattach(cwpr);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s\n", receive);
		lfsend(childRead_parentWrite, father1, strlen(father1) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s\n", receive);
		lfsend(childRead_parentWrite, father21, strlen(father21) + 1);
		lfsend(childRead_parentWrite, father22, strlen(father22) + 1);
		lfsend(childRead_parentWrite, father23, strlen(father23) + 1);
		lfsend(childRead_parentWrite, father24, strlen(father24) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s\n", receive);
		lfsend(childRead_parentWrite, father31, strlen(father31) + 1);
		lfsend(childRead_parentWrite, father32, strlen(father32) + 1);
		lfsend(childRead_parentWrite, father33, strlen(father33) + 1);
		lfsend(childRead_parentWrite, father34, strlen(father34) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s\n", receive);
		lfsend(childRead_parentWrite, father4, strlen(father4) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s\n", receive);
		lfsend(childRead_parentWrite, father5, strlen(father5) + 1);
		lfsend(childRead_parentWrite, father6, strlen(father6) + 1);
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
	}
	printf("Fork It Once Test Ended\n\n");
	return 0;
}

int joinLeaveTest() {
	int queue_ids[10];
	int success;

	printf("Join-Leave Test Started\n");
	queue_ids[0] = lfattach("zero");
	success = (queue_ids[0] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[1] = lfattach("un");
	success = (queue_ids[1] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[2] = lfattach("deux");
	success = (queue_ids[2] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[3] = lfattach("trois");
	success = (queue_ids[3] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[8] = lfattach("quatre");
	success = (queue_ids[4] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[8] = lfattach("cinq");
	success = (queue_ids[5] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[8] = lfattach("six");
	success = (queue_ids[6] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[8] = lfattach("sept");
	success = (queue_ids[7] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[8] = lfattach("huit");
	success = (queue_ids[8] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[9] = lfattach("neuf");
	success = (queue_ids[9] >= 0);
	printf("[%d] Queue Attach", success);

	success = lfleave(queue_ids[1]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[3]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[5]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[7]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[9]);
	printf("[%d] Queue Leave", success);

	queue_ids[1] = lfattach("dix");
	success = (queue_ids[1] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[3] = lfattach("onze");
	success = (queue_ids[3] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[5] = lfattach("douze");
	success = (queue_ids[5] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[5] = lfattach("treize");
	success = (queue_ids[7] >= 0);
	printf("[%d] Queue Attach", success);
	queue_ids[5] = lfattach("quatorze");
	success = (queue_ids[9] >= 0);
	printf("[%d] Queue Attach", success);

	success = lfsend(queue_ids[0], "0", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[1], "1", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[2], "2", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[3], "3", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[4], "4", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[5], "5", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[6], "6", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[7], "7", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[8], "8", 1);
	printf("[%d] Message Send", success);
	success = lfsend(queue_ids[9], "9", 1);
	printf("[%d] Message Send", success);

	success = lfleave(queue_ids[0]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[1]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[2]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[3]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[4]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[5]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[6]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[7]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[8]);
	printf("[%d] Queue Leave", success);
	success = lfleave(queue_ids[9]);
	printf("[%d] Queue Leave", success);

	success = lfsend(queue_ids[0], "0", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[1], "1", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[2], "2", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[3], "3", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[4], "4", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[5], "5", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[6], "6", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[7], "7", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[8], "8", 1);
	printf("[%d] Queue Send Fail", !success);
	success = lfsend(queue_ids[9], "9", 1);
	printf("[%d] Queue Send Fail", !success);

	return 0;
}

int pluralTest() {
	int success;
	int q1, q2, q3;
	char msg1[] = "8 chars";
	char msg2[] = "12 chars :D";
	char msg3[] = "13 chars :-(";
	char rcv[13];
	q1 = lfattach("uno");
	q2 = lfattach("dos");
	q3 = lfattach("tres");

	println("Plural Test Started");
	if(q1 >= 0 && q2 >= 0 && q3 >= 0) {
		lfsend(q1, msg1, 8);
		lfsend(q2, msg2, 12);
		lfsend(q3, msg3, 13);
		lfsend(q1, msg2, 12);
		lfsend(q2, msg3, 13);
		lfsend(q3, msg1, 8);
		lfsend(q1, msg3, 13);
		lfsend(q2, msg1, 8);
		lfsend(q3, msg2, 12);

		// From queue 1
		lfreceive(q1, rcv, 8);
		success = strcmp(rcv, msg1);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg1);

		lfreceive(q1, rcv, 12);
		success = strcmp(rcv, msg2);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg2);

		lfreceive(q1, rcv, 13);
		success = strcmp(rcv, msg3);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg3);

		// From queue 2
		lfreceive(q2, rcv, 12);
		success = strcmp(rcv, msg2);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg2);

		lfreceive(q2, rcv, 13);
		success = strcmp(rcv, msg3);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg3);

		lfreceive(q2, rcv, 8);
		success = strcmp(rcv, msg1);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg1);

		// From queue 3
		lfreceive(q3, rcv, 13);
		success = strcmp(rcv, msg3);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg3);

		lfreceive(q3, rcv, 8);
		success = strcmp(rcv, msg1);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg1);

		lfreceive(q3, rcv, 12);
		success = strcmp(rcv, msg2);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg2);

		printf("Plural Test End\n\n");
		return 0;
	}
	printf("Plural Test Error'd\n\n");
	return -1;
}

int boundsTest() {
	int success;
	int queue_id;
	char * rcv;
	char msg[] = "8 chars";
	rcv = malloc(sizeof(char) * 8);
	queue_id = lfattach("stuff");

	println("Bound Test Start\n");
	if(queue_id >= 0) {
		// Write 8, Read 4
		printf("\nWrite 8 - Read 4\n");
		success = lfsend(queue_id, msg, 8);
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 4);
		printf("[%d] Read 4 chars\n", success - 4);

		success = strncmp(msg, rcv, 4);
		printf("[%d] Contents match\n", success);

		// Write 8, Try read 12, Read 8
		printf("\nWrite 8 - Try read 12 - Read 8\n");
		success = lfsend(queue_id, msg, 8);
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 12);
		printf("[%d] Read 12 chars (actually, just 8)\n", success - 8);

		success = strcmp(msg, rcv);
		printf("[%d] Contents match\n", success);

		// Write 8, Read 8
		printf("\nWrite 8 - Read 8\n");
		success = lfsend(queue_id, msg, 8);
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 8);
		printf("[%d] Read 8 chars\n", success - 8);

		success = strcmp(msg, rcv);
		printf("[%d] Contents match\n", success);
		printf("Bound Test End\n\n");
		return 0;
	}

	printf("Bound Test Error'd\n\n");
	return -1;
}

int simpleTest() {
	int queue_id;
	char * msg;
	msg = malloc(sizeof(char)*6);

	queue_id = lfattach("stuff");

	printf("Simple Test Started\n");
	if(queue_id != -1) {
		lfsend(queue_id, "stuff", 6);
		printf("Sent message to %d\n", queue_id);

		lfreceive(queue_id, msg, 6);

		printf("Received %s\n", msg);

		lfdestroy(queue_id);
	} else {
		printf("Failed to attach.\n");
	}
	printf("Simple Test Ended\n\n");

	return 0;
}

void println(char * string) {
	printf("%s\n", string);
}

