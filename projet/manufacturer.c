#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <string.h>

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

int id;
int prod_speed;
struct product* warehouse_memory;
volatile sig_atomic_t warehouse;

int prod_type;
char* descr;
int volume;

/* PRODUCT */
struct product {
    int prod_type;              /* between 0 to 5 */
    char* descr;                /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};

/* manufacturer's variables */
volatile sig_atomic_t daytime = DAY;

/* Time handling */
/* each time this counter goes to 12, we switch states */
int counter = 0;

struct product produce(int prod_type, char* descr, int volume, int prod_speed){
    srand(time(0));             /* for serial number */
    sleep(prod_speed);          /* it takes time to produce something */
    struct product prod;
    prod.prod_type = prod_type;
    prod.descr = descr;
    prod.volume = volume;
    prod.serial_number = rand();
    return prod;
}

void handle_ok(int signum, siginfo_t* info, void* context) {
    /* stock_manager has picked up the product */
    warehouse = EMPTY;
    while (daytime == NIGHT) {
          printf("I'm sleeping...\n");
          pause();
    }
    produce(prod_type, descr, volume, prod_speed); /* TODO : how to get the params ? */
    warehouse = FULL;
    union sigval envelope;
    sigqueue(info->si_pid, SIGRT_READY, envelope);
    
}


void* time_management(void* args) {
    while(1) {
        sleep(12);
        daytime = daytime==NIGHT ? DAY : NIGHT; /* switch to night or day */
    }
}

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
    pthread_t time_thread;      /* this tread will handle the day/night cycle */
    pthread_create(&time_thread, NULL, time_management, NULL);
    /* pthread_t secondary; */
    /* pthread_t third; */
    /* pthread_t fourth; */
    /* pthread_t fifth; */
    
    //==========================================================================
    /* manufacturer property */
    id = 1;
    srand(time(0));
    prod_speed = rand()%5+1;     /* between [1, 5] */
    warehouse_memory; /* where we place our product */
    
    /* states */
    warehouse = EMPTY;
    
    /* product variables */
    /* prod number 1 */
    prod_type = 1;
    descr = "very cool product number 1 !";
    volume = 3;

    /* ---------------ROUTINE--------------- */
    /*
        We allocate memory for the product.
    */
    key_t ipc_key = ftok("./manufacturer1", 42); /* TODO: format this string */
    if (ipc_key == -1){
        perror("ftok");
        return EXIT_SUCCESS;
    }
    
    int shmid = shmget(ipc_key, sizeof(struct product), IPC_CREAT | 0666);
    if (shmid == -1){
        perror("shmget");
        return EXIT_FAILURE;
    }
    /* attach the memory */
    warehouse_memory = shmat(shmid, NULL, 0);
    if (warehouse_memory == (void*) - 1) {
        perror("shmat");
        return EXIT_FAILURE;
    }
    
    while (daytime == NIGHT) {
        printf("Manufacturer %d is spleeping...\n", id);
        pause();
    }
    // Warehouse is empty when nothing has been produced yet.
    produce(prod_type, descr, volume, prod_speed);
    // Warehouse is now full because something has been created.
    warehouse = FULL;
    union sigval envelope;
    pid_t pid = 4;
    sigqueue(pid, SIGRT_READY, envelope); //TODO changer pour le pid du gestionnaire O3O
}


//==========================================================================
// Threads initialization
/* void* init2(void* args) { */
/*     /\* manufacturer property *\/ */
/*     int id = 2; */
/*     srand(time(0)) */
/*     int prod_speed = rand(4)+1;     /\* between [1, 5] *\/ */
/*     struct product* warehouse_memory; /\* where we place our product *\/ */
    
/*     /\* states *\/ */
/*     volatile sig_atomic_t warehouse = EMPTY; */
    
/*     /\* product variables *\/ */
/*     /\* prod number 2 *\/ */
/*     int prod_type = 2; */
/*     char* descr = "very cool product number 2 !"; */
/*     int volume = 2; */

/*     /\* ---------------ROUTINE--------------- *\/ */
/*     /\* */
/*         We allocate memory for the product. */
/*     *\/ */
/*     key_t ipc_key = ftok("./manufacturer2", 42); /\* TODO: format this string *\/ */
/*     if (ipc_key == -1){ */
/*         perror("ftok"); */
/*         return EXIT_SUCCESS; */
/*     } */
    
/*     int shmid = shmget(ipc_key, sizeof(struct product), IPC_CREAT | 0666); */
/*     if (shmid == -1){ */
/*         perror("shmget"); */
/*         return EXIT_FAILURE; */
/*     } */
/*     /\* attach the memory *\/ */
/*     warehouse_memory = shmat(shmid, NULL, 0); */
/*     if (warehouse_memory == (void*) - 1) { */
/*         perror("shmat"); */
/*         return EXIT_FAILURE; */
/*     } */
/*     warehouse = EMPTY;          /\* starts empty *\/ */
/*     while (time == NIGHT) { */
/*         printf("Manufacturer %d is spleeping...\n", id); */
/*           pause(); */
/*     }// Warehouse is empty when nothing has been produced yet. */
/*     produce(prod_type, descr, volume, prod_speed); */
/*     // Warehouse is now full because something has been created. */
/*     warehouse = FULL; */
/*     union sigval envelope; */
/*     sigqueue(si_pid, SIGRT_READY, envelope); //TODO changer pour le pid du gestionnaire O3O */
/*     warehouse = FULL; */

/* } */

/* void* init3(void* args) { */
/*     /\* copy paste from init2 and adjust a view values *\/ */
/* } */

/* void* init4(void* args) { */
    
/* } */

/* void* init5(void* args) { */
    
/* } */

//==========================================================================
