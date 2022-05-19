#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

#include <pthread.h>

/* for multi-threading */
#define _GNU_SOURCE

/* States definition */
#define EMPTY 0
#define FULL 1

#define DAY 0
#define NIGHT 1

/* Signals definition */
#define SIGRT_READY (SIGRTMIN+1)
#define SIGRT_OK (SIGRTMIN+2)



/* PRODUCT */
typedef struct product {
    int prod_type;              /* between 0 to 5 */
    char* descr;                /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};

/* manufacturer's variables */
int id;
int prod_speed;
struct product* warehouse_memory;

/* product variables */
int prod_type;
char* descr;
int volume;

/* states */
volatile sig_atomic_t warehouse = EMPTY;
volatile sig_atomic_t time = DAY;

product produce(int prod_type, char* descr, int volume){
    /* struct product product; */
    /* product.prod_type = prod_type; */
    /* product.descr = descr; */
    /* product.volume = volume; */
    srand(time(0));
    /* product.serial_number = rand(); */
    return (product) {
        .prod_type = prod_type;
        .descr = descr;
        .volume = volume;
        .serial_number = rand();
    };
}

void handle_ok(int signum, siginfo_t* info, void* context) {
    /* stock_manager has pick up the product */
    warehouse = EMPTY;
    while (time == NIGHT) {
          printf("I'm sleeping...\n");
          pause();
    }
    produce();
    union sigval envelope;
    sigqueue(info->si_pid, SIGRT_READY, envelope); //TODO changer pour le pid du gestionnaire O3O
    warehouse = FULL;
}

//==========================================================================
// Threads initialization
void* init2(void* args) {
    id = 2;
    srand(time(0))
    prod_speed = rand(4)+1;     /* between [1, 5] */
    /* product affectation */
}

void* init3(void* args) {
    
}

void* init4(void* args) {
    
}

void* init5(void* args) {
    
}
//==========================================================================

int main() {

    //Should not ever receive messages before memory is created anyway.
    struct sigaction descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.sa_flags = SA_SIGINFO;
    descriptor.sa_sigaction = handle_ok;
    sigaction(SIGRT_OK, &descriptor, NULL);

    /* TODO:
       first we create 5 thread (one for each product),
       each thread will have a unique number between [1, 5] (product it will create).
       
    */

    //==========================================================================
    // THREADING
    pthread_t primary = pthread_self();
    pthread_t secondary;
    pthread_t third;
    pthread_t fourth;
    pthread_t fifth;
    
    //==========================================================================
    
    /*
        We allocate memory for the product.
    */
    key_t ipc_key = ftok("./manufacturer", 42); /* TODO: add id to memory name */
    if (ipc_key == -1){
        perror("ftok");
        /* return EXIT_SUCCESS; */
    }
    
    int shmid = shmget(ipc_key, sizeof(struct product), IPC_CREAT | 0666);
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
    warehouse = EMPTY;
    while (time == NIGHT) {
          printf("I'm sleeping...\n");
          pause();
    }// Warehouse is empty when nothing has been produced yet.
    produce();
    //Warehouse is now full because something has been created.
    warehouse = FULL;
    union sigval envelope;
    sigqueue(si_pid, SIGRT_READY, envelope); //TODO changer pour le pid du gestionnaire O3O
    warehouse = FULL;

}

