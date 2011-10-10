#include <unistd.h>
#include <qqservice.h>

int simpleTest();

int main(int argc, char * argv[]) {
	return 0;
}

int simpleTest() {
	int queue_id;
	char * msg;
	msg = malloc(sizeof(char)*6);

	queue_id = lfattach("stuff");

	if(queue_id != -1) {
		lfsend(queue_id, "stuff\0", 6);
		println("Sent message to %d", queue_id);

		lfreceive(queue_id, msg, 6);

		println("Received %s", msg);
	} else {
		println("Failed to attach.");
	}
}