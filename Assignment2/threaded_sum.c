#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <pthread.h> // for pthreads
#include <sys/time.h> // for gettimeofday

typedef struct _thread_data_t {
const int *data;
int startInd;
int endInd;
pthread_mutex_t *lock;
long long int *totalSum;
} thread_data_t;

void* arraysum(void*);

int readFile(char[], int[]);

int main(int argc, char* argv[])
{
    int threadNum = atoi(argv[2]);
    //printf("Thread num %d", threadNum);
    int nums[1000001];
    if(argc != 3)
    {
        printf("Incorrect number of parameters.\n");
        return -1;
    }

    int retVal = readFile(argv[1], nums);

    //printf("Read file \n");
    //printf("Retval: %d", retVal);
    if(retVal == -1)
    {
        return -1;
    }

    if(threadNum > retVal)
    {
        printf("Too many threads requested.\n");
        return -1;
    }

    long long int totalSum = 0;

    //printf("Check for time \n");
    struct timeval time1, time2;
    gettimeofday(&time1, NULL);
    //printf("Time 1 %ld\n", time1.tv_sec);

    pthread_mutex_t ptmt;
    //printf("Create\n");
    pthread_mutex_init(&ptmt, NULL);
    //printf("Mutex init\n");
    

    thread_data_t thread_data[threadNum];
    //printf("ThreadDataT\n");
    int splits = retVal / threadNum;

    for(int i = 0; i < threadNum; i++)
    {
        thread_data[i].data = nums;
        thread_data[i].startInd = splits*i; 
        thread_data[i].endInd = splits*(1+i);
        thread_data[i].lock = &ptmt;
        thread_data[i].totalSum = &totalSum;
    }
    thread_data[threadNum - 1].endInd = retVal;
    //printf("Loop\n");

    pthread_t thread[threadNum];
    for(int i = 0; i < threadNum; i++)
    {
        //printf("Thread %d\n", i+1);
        pthread_mutex_unlock(&ptmt);
        //printf("Pre-thread loop\n");
        pthread_create(&thread[i], NULL, arraysum, &thread_data[i]);
        pthread_join(thread[i], NULL);
        pthread_mutex_lock(&ptmt);
    }
    //printf("Out\n");
    
    gettimeofday(&time2, NULL);

    //printf("Time 2 %ld\n", time2.tv_sec);
    double final = (time2.tv_usec - time1.tv_usec) / 1000;
    //printf("LIF\n");
    printf("Final time: %f\n", final);
    printf("Final sum: %lld\n", totalSum);

    return 0;
}

int readFile(char file[], int name[])
{
    FILE *fptr;
    int i = 0;

    fptr = fopen(file, "r");
    //printf("Open file\n");
    if(fptr == NULL)
    {
        printf("File not found \n");
        return -1;
    }
    //printf("File found \n");
    while (fscanf(fptr, "%d", name + i) && !feof(fptr)) {
        i++;
        //printf("%d", i);
    }   
    fscanf(fptr, "%d", name + i);
    i++;
    fclose(fptr);
    return i;
}

void* arraysum(void* arg)
{
    thread_data_t *thread = (thread_data_t*) arg;
    long long int threadSum = 0;
    for(int i = thread->startInd; i < thread->endInd; i++)
    {
        
        threadSum += thread->data[i];
        //printf("Total sum: %lld\n", threadSum);
    }
    *thread->totalSum += threadSum;
    //printf("Thread sum: %lld\n", *thread->totalSum);
    return (void*)threadSum;
}