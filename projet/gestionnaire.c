#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>


/* Signals definitions */
#define SIGRT_READY (SIGRTMIN+1) /* received from manufacturer */
#define SIGRT_OK (SIGRTMIN+2)   /* to be sent to manufacturer */


/* PRODUCT */
struct product {
    int prod_type;              /* between 0 to 5 */
    char* descr;                /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};



int total_volume;               /* the total volume available in the stock */

int[5] prod_space = {10, 10, 10, 10, 10};              /* space allowed for a specific product.
                                                          if prod_space[3] == 4, it means that there can ba
                                                          at most 4 product of type 4 in stock
                                                       */
int[5] prod_volume;             /* volume taken by a prod.
                                 if prod_volume[1] == 2, it means that product of type 1
                                 has a volume of 2
                                */


struct product* warehouse_memory;

void handle_ready(int signum, siginfo_t* info, void* context) {
    /* id of manufacturer */
    int man_id = info->si_value.sival_int;
    
    /* retrieve the product */
    key_t ipc_key = ftok("./manufacturer1", 42); /* TODO: format this string */
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
}

void handle_prod_collection(int signum, siginfo_t* info, void* context) {
    /* we receive a ready */
    int man_id = info->si_value.sival_int;
    /* check if we have the place to stock it */
    /* accesss the warehouse */

    /* add the product  */
}

int main()
{
    /* encode products values */
    prod_volume = {2, 3, 1, 5, 4};
    prod_space = {10, 10, 10, 10, 10};
    total_volume = 2*10 + 3*10 + 1*10 + 5*10 + 4*10;
    products = {0, 0, 0, 0, 0};
    
    /* handle ready signal (when a manufacturer has built something) */

    /* creates a message queue to handle commands */

    /* product stock */
    struct product[prod_space[0]] prod1;
    struct product[prod_space[1]] prod2;
    struct product[prod_space[2]] prod3;
    struct product[prod_space[3]] prod4;
    struct product[prod_space[4]] prod5;
    
    return 0;
}
