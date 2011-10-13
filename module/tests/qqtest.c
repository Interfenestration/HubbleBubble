#include <unistd.h>
#include <sys/qqservice.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int joinLeaveDestroyTest();
int pluralTest();
int boundsTest();
int simpleTest();

int main(int argc, char * argv[]) {
	simpleTest();
	joinLeaveDestroyTest();
	pluralTest();
	boundsTest();

	return 0;
}

/* Testa a utilizacao geral das queues e unico a este teste, envio de mensagens para queues existentes a que nao está attach. */
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

/* Testa o envio de mensagens e a recepcao tal como a ordenacao nas queues */
int pluralTest() {
	int success;
	int q1;
	int q2;
	int q3;
	char * msg1 = "8 chars";
	char * msg2 = "12 chars bs";
	char * msg3 = "13 chars bcb";
	char * rcv = calloc(sizeof(char), 20);
	q1 = lfattach("_uno_", strlen("_uno_"));
	q3 = lfattach("_tres_", strlen("_tres_"));
	q2 = lfattach("_dos_", strlen("_dos_"));

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
		lfreceive(q1, rcv, strlen(msg1));
		success = strcmp(rcv, msg1);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg1);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q1, rcv, strlen(msg2));
		success = strcmp(rcv, msg2);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg2);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q1, rcv, strlen(msg3));
		success = strcmp(rcv, msg3);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg3);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		// From queue 2
		lfreceive(q2, rcv, strlen(msg2));
		success = strcmp(rcv, msg2);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg2);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q2, rcv, strlen(msg3));
		success = strcmp(rcv, msg3);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg3);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q2, rcv, strlen(msg1));
		success = strcmp(rcv, msg1);
		printf("[%d] From q2: %s == %s\n", success, rcv, msg1);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		// From queue 3
		lfreceive(q3, rcv, strlen(msg3));
		success = strcmp(rcv, msg3);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg3);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q3, rcv, strlen(msg1));
		success = strcmp(rcv, msg1);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg1);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		lfreceive(q3, rcv, strlen(msg2));
		success = strcmp(rcv, msg2);
		printf("[%d] From q1: %s == %s\n", success, rcv, msg2);
		free(rcv);
		rcv = calloc(sizeof(char), 20);

		printf("Plural Test End\n\n");
		return 0;
	}
	printf("Plural Test Error'd\n\n");
	return -1;
}

/* Testa o envio de mensagens e a recepcao parcial e recepcao pedindo mais que a mensagem tem. */
int boundsTest() {
	int success;
	int queue_id;
	char * rcv;
	char * msg = "8 chars";
	rcv = calloc(sizeof(char), 8);
	queue_id = lfattach("_stuff_", strlen("_stuff_"));

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
		printf("[%d] Read 12 chars (actually, just 8)\n", success - 7);

		success = strcmp(msg, rcv);
		printf("[%d] Contents match\n", success);

		// Write 8, Read 8
		printf("\nWrite 8 - Read 8\n");
		success = lfsend(queue_id, msg, strlen(msg));
		printf("[%d] Send 8-char message\n", success);

		success = lfreceive(queue_id, rcv, 8);
		printf("[%d] Read 8 chars\n", success - 7);

		success = strcmp(msg, rcv);
		printf("[%d] Contents match\n", success);
		printf("Bound Test End\n\n");
		return 0;
	}

	printf("Bound Test Error'd\n\n");
	return -1;
}

/* Teste mais simples para testar funcionalidade basica do modulo */
int simpleTest() {
	int queue_id;
	int read_size;
	char * msg;
	msg = calloc(sizeof(char),10);

	queue_id = lfattach("stuff", strlen("stuff"));

	printf("Simple Test Started\n");
	if(queue_id >= 0) {
		read_size = lfsend(queue_id, "stuff", strlen("stuff"));
		printf("Sent message to %d; return code:%d\n", queue_id, read_size);

		read_size = lfreceive(queue_id, msg, 6);

		printf("Received size:%d :: %s\n", read_size, msg);

		lfdestroy(queue_id);
	} else {
		printf("Failed to attach.Error code: %d\n", queue_id);
	}
	printf("Simple Test Ended\n\n");

	return 0;
}

