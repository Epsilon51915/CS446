
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include <signal.h>

#include <ucontext.h>

#include <stdatomic.h>

#include "modify_value.h"

#define REPS 100000

int counter = 0;

int task_1_finished = 0;
int task_2_finished = 0;

void task_1_func() {
    printf("Task 1 started.\n");

    for (int i=0; i<REPS; ++i) {
		

        // implement spin-guard lock()-ing //


        while (rand() % 8 != 0) {}  // short, random delay to prevent loop unrolling

        if (i % 1000 == 0) { printf("Task 1 running.\n"); }  // debug print every 1000 reps

        modify_value_up(&counter);


        // implement spin-guard unlock()-ing //


    }
    
    printf("Task 1 finished.\n");
    task_1_finished = 1;
}

void task_2_func() {
    printf("Task 2 started.\n");

    for (int i=0; i<REPS; ++i) {
		

        // implement spin-guard lock()-ing //


        while (rand() % 8 != 0) {}  // short, random delay to prevent loop unrolling

        if (i % 1000 == 0) { printf("Task 2 running.\n"); }  // debug print every 1000 reps

        modify_value_down(&counter);


        // implement spin-guard unlock()-ing //


    }
    
    printf("Task 2 finished.\n");
    task_2_finished = 1;
}


void time_slice_expired_handler(int signal) {
    
    printf("\t\tTIME SLICE EXPIRED\n");

    // implement simple scheduling between tasks //
}


int main(int argc, char *argv[]){

    srand(time(NULL));  // initialize random number generator

    // implement task1 and task2 ucontext setup //

    // implement interval timer setup //
    struct itimerval timer;
    timer.it

    setitimer(ITIMER_REAL, );

    printf("Main started.\n");
	
    while (!task_1_finished || !task_2_finished) {
        printf("\t\tTasks 1 && 2 not both Finished yet - Waiting...\n");
		
		// implement any other task scheduling operations you might need here //
		
        pause();  // pause the main Thread, to wait for delivery of the the next timer-based signal 
    }
	
    printf("Main: Finished. Final counter: %d\n", counter);
 
    return 0;  
}
