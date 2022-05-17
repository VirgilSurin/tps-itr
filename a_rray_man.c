#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#define SIGRT_DONE (SIGRTMIN+1)
#define SIGRT_OK (SIGRTMIN+2)

#define STARTING 0
#define PROCESSING 1
#define WAITING 2
#define TERMINATING 3

/* starting state */
volatile sig_atomic_t state = STARTING;
int tab_size = sizeof(signed int)*65536;
signed int* shared_mem;

signed int* create_table(){
    // Use current time as seed for random generator
    srand(time(0));

    signed int* table = malloc(sizeof(signed int) * 65536);
 
    for(int i = 0; i<65536; i++){
        table[i] = rand();
    }
    return table;
}

int wr_table(){
    signed int* table = create_table();

    key_t ipc_key = ftok("malenia", 42);
    if (ipc_key == -1){
        perror("ftok");
        return EXIT_SUCCESS;
    }

    int shmid = shmget(ipc_key, 65536*sizeof(signed int) + 2*sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        return EXIT_FAILURE;
    }

    shared_mem = shmat(shmid, NULL, 0);
    if (shared_mem == (void*) - 1)
    {
        perror("shmat");
        return EXIT_FAILURE;
    }
    else{
        for(int i = 0; i<65536; i++){
            
            shared_mem[i] = table[i];
        }
    }

    shmdt( shared_mem );
    return EXIT_SUCCESS;
}


void handle_done(int signum, siginfo_t* info, void* context) {
    printf("DONE time\n");
    state = PROCESSING;
    
    /* re-read in the shared memory */
    key_t ipc_key = ftok("./malenia", 42);
    if (ipc_key == -1){
        perror("ftok");
        /* return EXIT_SUCCESS; */
    }
    
    int shmid = shmget(ipc_key, tab_size+2*sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        /* return EXIT_FAILURE; */
    }
    /* attach the memory */
    shared_mem = shmat(shmid, NULL, 0);
    if (shared_mem == (void*) - 1) {
        perror("shmat");
        /* return EXIT_FAILURE; */
    }
    
    for (int i = 0; i < 65535; i++)
    {
        printf("%d, ", shared_mem[i]);
        if (!(shared_mem[i] < shared_mem[i+1]))
        {
            /* incorrectly sorted, recalling master */
            union sigval envelope;
            pid_t master_pid = shared_mem[0];
            sigqueue(master_pid, SIGRT_OK, envelope);
            /* return; */
        }
        
    }
    /* release the memory */
    shmdt(shared_mem);
    /* back to waiting */
    state = TERMINATING;
}

int main(int argc, char const *argv[]) {
    /* use -pthread when compilling ! */

    /* signal association */
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_done;
    sigaction(SIGRT_DONE, &descriptor, NULL);
    
    sem_t* sema = NULL;
    do{
        sema = sem_open("/memory_semaphore" , O_CREAT, 0600, 1);
    }while (sema == SEM_FAILED);
    /* sem_wait(sema); //useless here because semaphore cannot be created if another process is already working anyway? */
    state = PROCESSING;

    /* start of write table */
    signed int* table = create_table();

    key_t ipc_key = ftok("./malenia", 42);
    if (ipc_key == -1){
        perror("ftok");
        return EXIT_SUCCESS;
    }
    
    int shmid = shmget(ipc_key, 65536*sizeof(signed int) + 2*sizeof(pid_t), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        return EXIT_FAILURE;
    }
    
    shared_mem = shmat(shmid, NULL, 0);
    if (shared_mem == (void*) - 1)
    {
        perror("shmat");
        return EXIT_FAILURE;
    }
    else{
        int* tab = shared_mem + 2*sizeof(pid_t);
        for(int i = 0; i<65536; i++){
            
            tab[i] = table[i];
        }
    }
    /* end of write table */
    pid_t master_pid = shared_mem[0];
    shared_mem[1] = getpid();
    shmdt(shared_mem);
    
    /* process done, send signal */
    union sigval envelope;
    printf("sending ok to %d\n", master_pid);
    sigqueue(master_pid, SIGRT_OK, envelope);
    /* back to waiting */
    state = WAITING;
    
    while (state == WAITING) {
        printf("waiting...\n");
        pause();
    }
    printf("YOUPI!\n");
    sem_post(sema);

    sem_unlink("/memory_semaphore");

    return 0;
}


