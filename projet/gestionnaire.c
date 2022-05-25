#include <bits/types/sigevent_t.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <mqueue.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>


/* Signals definitions */
#define SIGRT_READY (SIGRTMIN+1) /* received from manufacturer */
#define SIGRT_OK (SIGRTMIN+2)   /* to be sent to manufacturer */
#define SIGRT_QUEUE (SIGRTMIN+3) /*something arrived in the message queue*/


/* PRODUCT */
struct product {
    int prod_type;              /* between 0 to 5 */
    char* descr;                /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};

mqd_t queue;

int total_volume;               /* the total volume available in the stock */

int prod_space[5] = {10, 10, 10, 10, 10};              /* space allowed for a specific product.
                                                       if prod_space[3] == 4, it means that there can ba
                                                       at most 4 product of type 4 in stock
                                                    */
int prod_volume[5] = {2, 3, 1, 5, 4};             /* volume taken by a prod.
                                                     if prod_volume[1] == 2, it means that product of type 1
                                                     has a volume of 2
                                                  */


/* product stock */
struct product prod1[10];
struct product prod2[10];
struct product prod3[10];
struct product prod4[10];
struct product prod5[10];

struct product* warehouse_memory;
int* pid_memory;

struct product remove_last_prod(int prod_type) {
    switch (prod_type) {
    case 1: {
        return prod1[10-prod_space[prod_type]-1];
    }                                        
    case 2: {                                
        return prod2[10-prod_space[prod_type]-1];
    }                                        
    case 3: {                                
        return prod3[10-prod_space[prod_type]-1];
    }                                        
    case 4: {                                
        return prod4[10-prod_space[prod_type]-1];
    }                                        
    case 5: {                                
        return prod5[10-prod_space[prod_type]-1];
    }
    }
    struct product p;       /* void prod, should never be here, just for the warning */
    return p;
}


void handle_ready(int signum, siginfo_t* info, void* context) {
    /* id of manufacturer */
    int man_id = info->si_value.sival_int;
        
    /* retrieve the product */
    key_t ipc_key = ftok(&"./manufacturer1" [man_id], 42);
    if (ipc_key == -1){
        perror("ftok");
    }
    
    int shmid = shmget(ipc_key, sizeof(int)+sizeof(struct product), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
    }
    /* attach the memory */
    warehouse_memory = shmat(shmid, NULL, 0);
    if (warehouse_memory == (void*) - 1) {
        perror("shmat");
    }
}

void handle_queue(int signum, siginfo_t* info, void* context) {
    /* there is an order in the mq */
    char* order = malloc(sizeof(char)*5);
    char* ptr_order = order;
    ssize_t receive = mq_receive(queue, order, sizeof(int)*5, 0);
    if (receive == -1) { printf("receive\n"); }

    int required[5];
    int sum = 0;
    int flag = 0;
    for (int i = 0; i < 5; i++) {
        required[i] = atoi(order[i]);
        if (10-prod_space[i] <= required[i]) {
            /* we don't have enough */
            flag = 1;
            break;
        }
        sum += atoi(order[i]);
    }
    if (flag == 0) {
        /* we have everything for the roder */
        struct product prod[50];
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < required[i]; j++) {
                switch (i) {
                case 1: {
                    prod[i+j] = remove_last_prod(i);
                    break;
                }
                case 2: {
                    prod[i+j] = remove_last_prod(i);
                    break;
                }
                case 3: {
                    prod[i+j] = remove_last_prod(i);
                    break;
                }
                case 4: {
                    prod[i+j] = remove_last_prod(i);
                    break;
                }
                case 5: {
                    prod[i+j] = remove_last_prod(i);
                    break;
                }
                }
                prod_space[i]++;
            }
        }
        /* TODO : place the prod in client's warehouse */
        
        mq_close(queue);
    
    }
}
void handle_prod_collection(int signum, siginfo_t* info, void* context) {
    /* we receive a ready */
    int man_id = info->si_value.sival_int;
    /* check if we have the place to stock it */
    if (prod_space[man_id] > 0) {
        /* there is space for it */
        /* accesss the warehouse */
        key_t ipc_key = ftok(&"./manufacturer1" [man_id], 42);
        if (ipc_key == -1){
            perror("ftok");
        }
    
        int shmid = shmget(ipc_key, sizeof(struct product), IPC_CREAT | 0666);
        if (shmid == -1){
            perror("shmget");
        }
        /* attach the memory */
        warehouse_memory = shmat(shmid, NULL, 0);
        if (warehouse_memory == (void*) - 1) {
            perror("shmat");
        }
        struct product prod = warehouse_memory[0];
        switch (prod.prod_type) {
        case 1: {
            prod1[10-prod_space[prod.prod_type]--] = prod;
            break;
        }
        case 2: {
            prod2[10-prod_space[prod.prod_type]--] = prod;
            break;
        }
        case 3: {
            prod3[10-prod_space[prod.prod_type]--] = prod;
            break;
        }
        case 4: {
            prod4[10-prod_space[prod.prod_type]--] = prod;
            break;
        }
        case 5: {
            prod5[10-prod_space[prod.prod_type]--] = prod;
            break;
        }
        default:
            perror(&"INVALID PROD TYPE !" [prod.prod_type]);
            break;
        }
        /* send OK to manufacturer */
        union sigval envelope;
        sigqueue(info->si_pid, SIGRT_OK, envelope);
        printf("recieved : \n");
        printf("type  : %d\ndescr  : %s\nvolume : %d\ns_n    : %d\n",
               prod.prod_type, prod.descr, prod.volume, prod.serial_number);
        shmdt(warehouse_memory);
    }
    printf("Storage full !\n");
    /* no space, we leave */
    
}

int main()
{
    /* create the queue */
    queue = mq_open ("/message-queue", O_CREAT | O_RDONLY );
    if (queue == -1) { perror ("mq_open"); return EXIT_FAILURE ; }

    /* handler for queue */
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_queue;
    sigaction(SIGRT_QUEUE, &descriptor, NULL);

    /* queue notification */
    struct sigevent sigevent;
    memset(&sigevent, 0, sizeof(sigevent));
    sigevent.sigev_signo = SIGRT_QUEUE;
    mq_notify(queue, &sigevent);
    
    /* encode products values */
    total_volume = 2*10 + 3*10 + 1*10 + 5*10 + 4*10;
    
    /* creates a memory for our pid */
    key_t ipc_key = ftok("./m_pid", 42);
    if (ipc_key == -1){
        perror("ftok");
    }
    
    int shmid = shmget(ipc_key, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
    }
    /* attach the memory */
    pid_memory = shmat(shmid, NULL, 0);
    if (pid_memory == (void*) - 1) {
        perror("shmat");
    }
    pid_memory[0] = getpid();
    shmdt(pid_memory);          /* detach memory */
    
    /* handle ready signal (when a manufacturer has built something) */
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_prod_collection;
    sigaction(SIGRT_READY, &descriptor, NULL);
    
    /* creates a message queue to handle commands */
    mq_close(queue);
    mq_unlink("/message-queue");
    return 0;
}
