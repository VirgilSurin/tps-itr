#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>


#define SIGRT_DONE (SIGRTMIN+1)
#define SIGRT_OK (SIGRTMIN+2)

#define STARTING 0
#define WAITING 1
#define PROCESSING 2
#define TERMINATING 3

/* starting state */
volatile sig_atomic_t state = STARTING;
int* segment;
int shmid;
int tab_max_size = sizeof(signed int)*65536;

int compare(const void* a, const void* b) {
    /* used to compare two numbers in the qsort function */
    int x = *((int*) a);
    int y = *((int*) b);
    if (x == y) {return 0;}
    if (x < y) {return -1;}
    return 1;
}

void sort(signed int* tab, int tab_len) {
    /* sort the table */
    qsort(tab, tab_len, sizeof(signed int), compare);
}

void handle_ok(int signum, siginfo_t* info, void* context) {
    state = PROCESSING;
    
    /* process the array */
    shmat(shmid, NULL, 0);                  /* read the tab */
    int tab_len = segment[1];               /* the pid of the client */
    int* tab = segment + 2*sizeof(int);   /* shift to the second case, because the first one is for the pid */
    sort(tab, tab_len);                     /* process the tab */
    shmdt(segment);                         /* detach the shared memory */
    /* process done, send signal */
    union sigval envelope;
    sigqueue(info->si_pid, SIGRT_DONE, envelope);
    /* back to waiting */
    state = WAITING;
}

void handle_death(int signum, siginfo_t* info, void* context) {
    /* we properly kill the shared memory */
    shmdt(segment);
    state = TERMINATING;
}

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    printf("my pid : %d\n", pid);
    
    /* handle signals */
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_ok;
    sigaction(SIGRT_OK, &descriptor, NULL);
    
    
    /* creates the shared memory */
    key_t ipc_key = ftok("./malenia", 42);
    if (ipc_key == -1){
        perror("ftok");
        return EXIT_SUCCESS;
    }
    
    shmid = shmget(ipc_key, tab_max_size+2*sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        return EXIT_FAILURE;
    }
    
    segment = shmat(shmid, NULL, 0);
    if (segment == (void*) - 1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    /* write our pid on the first case */
    segment[0] = pid;
    shmdt(segment);
    
    /* WAITING for arrays to sort */
    state = WAITING;
    /* now we wait */
    printf("ready...\n");
    while (state == WAITING) {
        pause();
    }
    printf("This is the end\n");
    return 0;
}

