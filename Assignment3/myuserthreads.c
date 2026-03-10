// Lock before modifying values held between multiple contexts
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
//#define STACK_SIZE sysconf(_SC_SIGSTKSZ)
#define STACK_SIZE 8192


int counter = 0;

int task_1_finished = 0;
int task_2_finished = 0;
struct ucontext_t main_context;
struct ucontext_t task1_context;
struct ucontext_t task2_context;

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
		
        //printf("2i: %d", i);
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

    struct ucontext_t current_context;
    getcontext(&current_context);

    if(task_1_finished == 1 && task_2_finished == 1)
    {
        setcontext(&main_context);
    }
    if(task_1_finished == 1)
    {
        printf("T1 done");
        swapcontext(&current_context, &task2_context);
    }
    if(task_2_finished == 1)
    {
        swapcontext(&current_context, &task1_context);
    }
    if(&current_context.uc_stack.ss_sp == &task1_context.uc_stack.ss_sp)
    {
        printf("1 to 2");
        swapcontext(&task1_context, &main_context);
    }
    if(&current_context.uc_stack.ss_sp == &task2_context.uc_stack.ss_sp)
    {
        printf("2 to 1");
        swapcontext(&task2_context, &main_context);
    }
}


int main(int argc, char *argv[]){

    srand(time(NULL));  // initialize random number generator

    // implement task1 and task2 ucontext setup //
    getcontext(&main_context);
    getcontext(&task1_context);
    getcontext(&task2_context);

    void* stack1 = malloc(STACK_SIZE);
    void* stack2 = malloc(STACK_SIZE);

    task1_context.uc_stack.ss_sp = stack1;
    task2_context.uc_stack.ss_sp = stack2;

    task1_context.uc_stack.ss_size = STACK_SIZE;
    task2_context.uc_stack.ss_size = STACK_SIZE;

    task1_context.uc_link = &main_context;
    task2_context.uc_link = &main_context;

    makecontext(&task1_context, task_1_func, 0);
    makecontext(&task2_context, task_2_func, 0);

    sigemptyset(&task1_context.uc_sigmask);
    sigemptyset(&task2_context.uc_sigmask);

    // implement interval timer setup //
    
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000;
    timer.it_interval.tv_sec = 0;
    // Interval of 1 ms (1000 us)
    timer.it_interval.tv_usec = 1000;

    setitimer(ITIMER_REAL, &timer, NULL);

    signal(SIGALRM, time_slice_expired_handler);

    // begin operations //

    printf("Main started.\n");
	
    
    while (!task_1_finished || !task_2_finished) {
        printf("\t\tTasks 1 && 2 not both Finished yet - Waiting...\n");
		
		// implement any other task scheduling operations you might need here //
		swapcontext(&main_context, &task1_context);
        swapcontext(&main_context, &task2_context);
        pause();  // pause the main Thread, to wait for delivery of the the next timer-based signal 
    }
	
    printf("Main: Finished. Final counter: %d\n", counter);

    free(stack1);
    free(stack2);
 
    return 0;  
}
