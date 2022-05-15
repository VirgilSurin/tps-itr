#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>

#define SIGRT_DONE (SIGRTMIN+1)
#define SIGRT_OK (SIGRTMIN+2)

#define STARTING 0
#define WAITING 1
#define PROCESSING 2
#define TERMINATING 3

/* starting state */
volatile sig_atomic_t state = STARTING;

signed int* shared_mem;

int main(int argc, char const *argv[])
{
    sem_t* sema = NULL;
    do{
        sema = sem_open("/memory_semaphore" , O_CREAT, 0600, 1);
    }while (sema == SEM_FAILED);
    sem_wait(sema); //useless here because semaphore cannot be created if another process is already working anyway?
    
    wr_table();
    
    
    state = WAITING;
    sem_post(sema);

    sem_unlink("/memory_semaphore");

    return 0;
}

signed int* create_table(){
        // Use current time as seed for random generator
    srand(time(0));

    signed int table[65536];
 
    for(int i = 0; i<65536; i++){
        table[i] = rand();
    }
    return table;
}

void wr_table(){
    signed int* table = create_table();

    key_t ipc_key = ftok('malenia', 42);
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
    }
    else{
        for(int i = 0; i<65536; i++){
            
            shared_mem[i] = table[i];
        }
    }

    shmdt( shared_mem );
}


