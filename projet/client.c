#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>

#include <mqueue.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

#define SIGRT_READY (SIGRTMIN+1)
#define SIGRT_ORDER (SIGRTMIN+2)


struct product* warehouse_memory;

int c_id;
int min_req_t;
int max_req_t;

/* PRODUCT */
struct product {
    int prod_type;              /* between 0 to 5 */
    char descr[256];            /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};

void handle_ready(int signum, siginfo_t* info, void* context){

    mqd_t queue = mq_open ( " / message - queue " , O_WRONLY );
    if ( queue == -1) { perror ( " mq_open " ); return EXIT_FAILURE ; }

    /* we randomly generate a table */
    // Use current time as seed for random generator
    srand(getpid());
    sleep(rand()%(max_req_t+min_req_t) + min_req_t);

    int order[5];

    for (int i = 0; i < sizeOf(order); i++)
    {
        order[i] = rand() % 10;
    }
    
    int status = mq_send ( queue , order , sizeOf(order) , 0);
    if ( status == -1) perror ( " mq_send " );

    mq_close ( queue );

    return EXIT_SUCCESS ;

    union sigval envelope;
    sigqueue(info->si_pid, SIGRT_READY, envelope);
}


int main() {

    //Should not ever receive messages before memory is created anyway.
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_ready;
    sigaction(SIGRT_READY, &descriptor, NULL);

    /* TODO:
       first we create 5 thread (one for each product),
       each thread will have a unique number between [1, 5] (product it will create).
       
    */

    /*
        We allocate memory for the product.
    */
    key_t ipc_key = ftok("./client", 42);
    if (ipc_key == -1){
        perror("ftok");
        /* return EXIT_SUCCESS; */
    }
    
    int shmid = shmget(ipc_key, sizeof(struct product)*50, IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        /* return EXIT_FAILURE; */
    }
    /* attach the memory */
    warehouse_memory = shmat(shmid, NULL, 0);
    if (warehouse_memory == (void*) - 1) {
        perror("shmat");
        /* return EXIT_FAILURE; */
    }


}
