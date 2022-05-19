#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

/* States definition */
#define EMPTY 0
#define FULL 1

#define DAY 0
#define NIGHT 1

/* Signals definition */
#define SIGRT_READY (SIGRTMIN+1)

/* PRODUCT */
struct product {
    int prod_type;              /* between 0 to 5 */
    char[256] descr;            /* depends on prod_type */
    int volume;                 /* depends on prod_type */
    int serial_number;          /* randomly generated */
};

/* manufacturer's variables */
int id;
int prod_time;
product* ready;
bool full;

/* states */
volatile sig_atomic_t warehouse = EMPTY;
volatile sig_atomic_t time = DAY;


void handle_ok(int signum, siginfo_t* info, void* context) {
    /* stock_manager has pick up the product */
    warehouse = EMPTY;
    while (time == NIGHT) {
          printf("I'm sleeping...\n");
          pause();
    }
    produce();
    union sigval envelope;
    sigqueue(info->si_pid, SIGRT_READY, envelope);
    warehouse = FULL;
}

int main() {

    /* TODO:
       first we create 5 thread (one for each product),
       each thread will have a unique number between [1, 5] (product it will create).
       
     */


    /* day time, work time */
    while (state == ACTIVE) {
        if (!full) {prod();} 
    }
    /* good night */
    while (state == SLEEP) {
        pause();
    }
    return 0;
}

