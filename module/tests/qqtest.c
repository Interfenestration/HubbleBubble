#include <unistd.h>
#include <sys/qqservice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int forkItTwice();
int for_me(char[], char);
int forkItOnce();
int joinLeaveDestroyTest();
int pluralTest();
int boundsTest();
int simpleTest();

int main(int argc, char * argv[]) {
	//simpleTest();
	//joinLeaveDestroyTest();
	forkItOnce();
	//forkItTwice();
	//pluralTest();
	//boundsTest();

	return 0;
}

int forkItTwice() {
	int queue_id;
	int success;
	char * msgs[9];
	char received[50];
	char * queue_name = "family";
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
					printf("Child says: Not for me... %s\n", received);
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
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
					printf("Parent says: Not for me... %s\n", received);
					lfsend(queue_id, received, 50);
				}
			}
			lfleave(queue_id);
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
	int size;
	char * crpw = "crpw";
	char * cwpr = "cwpr";
	char * son1 = "Hello, Parent";
	char * father1 = "Hello, Child";
	char * son2 = "Parent, where do Children come from?";
	char * father21 = "Well, you see Child...";
	char * father22 = "When a Process loves itself so much...";
	char * father23 = "It grabs a fork and...";
	char * father24 = "A Child is born!";
	char * son3 = "And how does the Parent know it's the Parent and the Child knows it's the Child?";
	char * father31 = "That's easy!";
	char * father32 = "Both check what the fork left behind...";
	char * father33 = "If what they see is nothing then they're the Child...";
	char * father34 = "If what the see is the Child then they're the Parent.";
	char * son4 = "Thank you, Parent.";
	char * father4 = "You're welcome, Child";
	char * son5 = "I hope you wait for me...";
	char * father5 = "I will...";
	char * father6 = "...Or not. You can be a zombie for all I care...";
	char * receive = malloc(sizeof(char) * 100);

	printf("Fork It Once Test Started\n");
	success = fork();
	if(success == -1) {
		printf("Fork Failed!");
	} else if(success == 0) { // Son
		childRead_parentWrite = lfattach(crpw, strlen(crpw));
		childWrite_parentRead = lfattach(cwpr, strlen(cwpr));
		printf("Child has attached\n");
		lfsend(childWrite_parentRead, son1, strlen(son1));
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father1
		lfsend(childWrite_parentRead, son2, strlen(son2));
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father21
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father22
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father23
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father24
		lfsend(childWrite_parentRead, son3, strlen(son3));
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father31
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father32
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father33
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father34
		lfsend(childWrite_parentRead, son4, strlen(son4));
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father4
		lfsend(childWrite_parentRead, son5, strlen(son5));
		lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father5
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
	} else { // Parent
		childRead_parentWrite = lfattach(crpw, strlen(crpw));
		childWrite_parentRead = lfattach(cwpr, strlen(cwpr));
		printf("Parent has attached");
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		lfsend(childRead_parentWrite, father1, strlen(father1) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		lfsend(childRead_parentWrite, father21, strlen(father21) + 1);
		lfsend(childRead_parentWrite, father22, strlen(father22) + 1);
		lfsend(childRead_parentWrite, father23, strlen(father23) + 1);
		lfsend(childRead_parentWrite, father24, strlen(father24) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		lfsend(childRead_parentWrite, father31, strlen(father31) + 1);
		lfsend(childRead_parentWrite, father32, strlen(father32) + 1);
		lfsend(childRead_parentWrite, father33, strlen(father33) + 1);
		lfsend(childRead_parentWrite, father34, strlen(father34) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		lfsend(childRead_parentWrite, father4, strlen(father4) + 1);
		lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		lfsend(childRead_parentWrite, father5, strlen(father5) + 1);
		lfsend(childRead_parentWrite, father6, strlen(father6) + 1);
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
	}
	printf("Fork It Once Test Ended\n\n");
	return 0;
}

int joinLeaveDestroyTest() {
	int queue_ids[10];
	int success;

	printf("Join-Leave Test Started\n");
	queue_ids[0] = lfattach("zero", strlen("zero"));
	success = !(queue_ids[0] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[1] = lfattach("un", strlen("un"));
	success = !(queue_ids[1] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[2] = lfattach("deux", strlen("deux"));
	success = !(queue_ids[2] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[3] = lfattach("trois", strlen("trois"));
	success = !(queue_ids[3] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[4] = lfattach("quatre", strlen("quatre"));
	success = !(queue_ids[4] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[5] = lfattach("cinq", strlen("cinq"));
	success = !(queue_ids[5] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[6] = lfattach("six", strlen("six"));
	success = !(queue_ids[6] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[7] = lfattach("sept", strlen("sept"));
	success = !(queue_ids[7] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[8] = lfattach("huit", strlen("huit"));
	success = !(queue_ids[8] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[9] = lfattach("neuf", strlen("neuf"));
	success = !(queue_ids[9] >= 0);
	printf("[%d] Queue Attach\n", success);

	success = lfleave(queue_ids[1]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[3]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[5]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[7]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[9]);
	printf("[%d] Queue Leave\n", success);

	queue_ids[1] = lfattach("dix", strlen("dix"));
	success = !(queue_ids[1] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[3] = lfattach("onze", strlen("onze"));
	success = !(queue_ids[3] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[5] = lfattach("douze", strlen("douze"));
	success = !(queue_ids[5] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[7] = lfattach("treize", strlen("treize"));
	success = !(queue_ids[7] >= 0);
	printf("[%d] Queue Attach\n", success);
	queue_ids[9] = lfattach("quatorze", strlen("quatorze"));
	success = !(queue_ids[9] >= 0);
	printf("[%d] Queue Attach\n", success);

	success = lfsend(queue_ids[0], "0", strlen("0"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[1], "1", strlen("1"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[2], "2", strlen("2"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[3], "3", strlen("3"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[4], "4", strlen("4"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[5], "5", strlen("5"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[6], "6", strlen("6"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[7], "7", strlen("7"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[8], "8", strlen("8"));
	printf("[%d] Message Send\n", success);
	success = lfsend(queue_ids[9], "9", strlen("9"));
	printf("[%d] Message Send\n", success);

	success = lfleave(queue_ids[0]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[1]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[2]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[3]);
	printf("[%d] Queue Leave\n", success);
	success = lfleave(queue_ids[4]);
	printf("[%d] Queue Leave\n", success);
	success = lfdestroy(queue_ids[5]);
	printf("[%d] Queue Destroy\n", success);
	success = lfdestroy(queue_ids[6]);
	printf("[%d] Queue Destroy\n", success);
	success = lfdestroy(queue_ids[7]);
	printf("[%d] Queue Destroy\n", success);
	success = lfdestroy(queue_ids[8]);
	printf("[%d] Queue Destroy\n", success);
	success = lfdestroy(queue_ids[9]);
	printf("[%d] Queue Destroy\n", success);

	success = lfsend(queue_ids[0], "0", strlen("0"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[1], "1", strlen("1"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[2], "2", strlen("2"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[3], "3", strlen("3"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[4], "4", strlen("4"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[5], "5", strlen("5"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[6], "6", strlen("6"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[7], "7", strlen("7"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[8], "8", strlen("8"));
	printf("[%d] Queue Send Fail\n", !success);
	success = lfsend(queue_ids[9], "9", strlen("9"));
	printf("[%d] Queue Send Fail\n", !success);

	return 0;
}

int pluralTest() {
	int success;
	int q1, q2, q3;
	char * msg1 = "8 chars";
	char * msg2 = "12 chars :D";
	char * msg3 = "13 chars :-(";
	char rcv[13];
	q1 = lfattach("uno", strlen("uno"));
	q2 = lfattach("dos", strlen("dos"));
	q3 = lfattach("tres", strlen("tres"));

	printf("Plural Test Started\n");
	if(q1 >= 0 && q2 >= 0 && q3 >= 0) {
		lfsend(q1, msg1, strlen(msg1));
		lfsend(q2, msg2, strlen(msg2));
		lfsend(q3, msg3, strlen(msg3));
		lfsend(q1, msg2, strlen(msg2));
		lfsend(q2, msg3, strlen(msg3));
		lfsend(q3, msg1, strlen(msg1));
		lfsend(q1, msg3, strlen(msg3));
		lfsend(q2, msg1, strlen(msg1));
		lfsend(q3, msg2, strlen(msg2));

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
	char * msg = "8 chars";
	rcv = malloc(sizeof(char) * 8);
	queue_id = lfattach("stuff", strlen("stuff"));

	printf("Bound Test Start\n");
	if(queue_id >= 0) {
		// Write 8, Read 4
		printf("\nWrite 8 - Read 4\n");
		success = lfsend(queue_id, msg, strlen(msg));
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 4);
		printf("[%d] Read 4 chars\n", success - 4);

		success = strncmp(msg, rcv, 4);
		printf("[%d] Contents match\n", success);

		// Write 8, Try read 12, Read 8
		printf("\nWrite 8 - Try read 12 - Read 8\n");
		success = lfsend(queue_id, msg, strlen(msg));
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 12);
		printf("[%d] Read 12 chars (actually, just 8)\n", success - 8);

		success = strcmp(msg, rcv);
		printf("[%d] Contents match\n", success);

		// Write 8, Read 8
		printf("\nWrite 8 - Read 8\n");
		success = lfsend(queue_id, msg, strlen(msg));
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
	int read_size;
	char * msg;
	msg = malloc(sizeof(char)*6);

	queue_id = lfattach("stuff", strlen("stuff"));

	printf("Simple Test Started\n");
	if(queue_id >= 0) {
		lfsend(queue_id, "stuff", strlen("stuff"));
		printf("Sent message to %d\n", queue_id);

		read_size = lfreceive(queue_id, msg, 6);

		printf("Received size:%d :: %s\n", read_size,msg);

		lfdestroy(queue_id);
	} else {
		printf("Failed to attach.Error code: %d\n", queue_id);
	}
	printf("Simple Test Ended\n\n");

	return 0;
}

