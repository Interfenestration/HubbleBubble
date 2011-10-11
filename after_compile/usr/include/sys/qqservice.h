#include <unistd.h>
#include <sys/syscall.h>

#define __NR_qqservice 325
#define __NR_qqservice_named 326
#define __NR_qqservice_named_attach 327

#define LF_LEAVE 1
#define LF_DESTROY 2
#define LF_SEND 1
#define LF_RECEIVE 2

#define lfsend(queue_id, msg, size) syscall(__NR_qqservice, LF_SEND, queue_id, msg, size);

#define lfreceive(queue_id, msg, size) syscall(__NR_qqservice, LF_RECEIVE, queue_id, msg, size);

#define lfattach(name) syscall(__NR_qqservice_named_attach, name, getpid());

#define lfleave(queue_id) syscall(__NR_qqservice_named, LF_LEAVE, queue_id, getpid());

#define lfdestroy(queue_id) syscall(__NR_qqservice_named, LF_DESTROY, queue_id, getpid());
