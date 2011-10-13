#include <unistd.h>
#include <sys/qqservice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Cria um filho que comunica com o pai utilizando duas queues */
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
	char * receive = calloc(sizeof(char), 100);

	printf("Fork It Once Test Started\n");
	success = fork();
	if(success == -1) {
		printf("Fork Failed!");
	} else if(success == 0) { // Son
		childRead_parentWrite = lfattach(crpw, strlen(crpw));
		childWrite_parentRead = lfattach(cwpr, strlen(cwpr));
		printf("Child has attached\n");
		lfsend(childWrite_parentRead, son1, strlen(son1));
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father1
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childWrite_parentRead, son2, strlen(son2));
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father21
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father22
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father23
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father24
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childWrite_parentRead, son3, strlen(son3));
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father31
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father32
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father33
		free(receive);
		receive = calloc(sizeof(char), 100);
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father34
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childWrite_parentRead, son4, strlen(son4));
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father4
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childWrite_parentRead, son5, strlen(son5));
		size = lfreceive(childRead_parentWrite, receive, 100);
		printf("Parent said: %s; size=%d\n", receive, size); // father5
		free(receive);
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
		exit(0);
	} else { // Parent
		childRead_parentWrite = lfattach(crpw, strlen(crpw));
		childWrite_parentRead = lfattach(cwpr, strlen(cwpr));
		printf("Parent has attached\n");
		size = lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childRead_parentWrite, father1, strlen(father1));
		size = lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childRead_parentWrite, father21, strlen(father21));
		lfsend(childRead_parentWrite, father22, strlen(father22));
		lfsend(childRead_parentWrite, father23, strlen(father23));
		lfsend(childRead_parentWrite, father24, strlen(father24));
		size = lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childRead_parentWrite, father31, strlen(father31));
		lfsend(childRead_parentWrite, father32, strlen(father32));
		lfsend(childRead_parentWrite, father33, strlen(father33));
		lfsend(childRead_parentWrite, father34, strlen(father34));
		size = lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		free(receive);
		receive = calloc(sizeof(char), 100);
		lfsend(childRead_parentWrite, father4, strlen(father4));
		size = lfreceive(childWrite_parentRead, receive, 100);
		printf("Child said: %s; size=%d\n", receive, size);
		free(receive);
		lfsend(childRead_parentWrite, father5, strlen(father5));
		lfsend(childRead_parentWrite, father6, strlen(father6));
		lfleave(childRead_parentWrite);
		lfleave(childWrite_parentRead);
	}
	printf("Fork It Once Test Ended\n\n");
	return 0;
}

int main(int argc, char * argv[]) {
	forkItOnce();

	return 0;
}

