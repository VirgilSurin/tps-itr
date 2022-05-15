#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>


#define SIGRT_DONE (SIGRTMIN+1)
#define SIGRT_OK (SIGRTMIN+2)

#define STARTING 0
#define WAITING 1
#define PROCESSING 2
#define TERMINATING 3

/* starting state */
volatile sig_atomic_t state = STARTING;

mqd_t queue;
unsigned_int* segment;
int shmid;
int tab_size = sizeof(signed int)*65536;

void hande_ok(int signum, siginfo_t* info, void* context) {
    state = PROCESSING;
    
    /* process the array */
    shmat(shmid, NULL, 0);               /* read the tab */
    int tab = segment + sizeof(pid_t);   /* shift to the second case, because the first one is for the pid */
    sort(tab, tab_size);                 /* process the tab */
    shmdt(segment);                      /* detach the shared memory */
    
    /* process done, send signal */
    union sigval envelope;
    sigqueue(pid, SIGRT_DONE, envelope)
    /* back to waiting */
    state = WAITING;
}

void handle_death(int signum, siginfo_t* info, void* context) {
    /* we properly kill the message queue and shared memory */
    mq_close(queue);
    mq_unlink("/wrote_queue");
    shmdt(segment);
    state = TERMINATING;
}

int main(int argc, char *argv[]) {
    
    pid_t pid = getpid();
    
    /* creates the shared memory */
    key_t ipc_key = ftok("malenia", 42);
    shmid = shmget(ipc_key, tab_size+2*sizeof(pid_t), IPC_CREAT | 0666);
    /* write our pid on the first case */
    shmat(shmid, NULL, 0);
    segment[0] = pid;
    shmdt(segment);
    
    /* creates the message queue */
    queue = mq_open("/wrote-queue" , O_CREAT | O_RDONLY );
    
    /* WAITING for arrays to sort */
    state = WAITING;
    /* only now we are ready to handle signals */
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_ok;
    sigaction(SIGRT_OK, &descriptor, NULL);

    /* now we wait */
    while (state == WAITING) {
        pause();
    }
    printf("This is the end");
    return 0;
}

int compare(const void* a, const void* b) {
    int x = *((int*) a);
    int y = *((int*) b);
    if (x == y) {return 0};
    if (x < y) {return -1};
    return 1;
}

void sort(signed int* tab, int size) {
    qsort(tab, size, sizeof(signed int), compare)
}
