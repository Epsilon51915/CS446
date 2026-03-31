# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <sys/stat.h>
# include <pthread.h> // for pthreads
# include <time.h> // for clock_gettime



typedef struct _thread_data_t {
int localTid;
const int *data;
int numVals;
pthread_mutex_t *lock;
long long int *totalSum;
} thread_data_t;

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <sys/mman.h>

/****************************************************************************************************************
**
**      START DATA FOR PRINT_PROGRESS
**
****************************************************************************************************************/
#define ANSI_COLOR_GRAY    "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

#define ANSI_COLOR_RESET   "\x1b[0m"

#define TERM_CLEAR() printf("\033[H\033[J")
#define TERM_GOTOXY(x,y) printf("\033[%d;%dH", (y), (x))

void print_progress(pid_t localTid, size_t value) {
        pid_t tid = syscall(__NR_gettid);

        TERM_GOTOXY(0,localTid+1);

	char prefix[256];
        size_t bound = 100;
        sprintf(prefix, "%d: %ld (ns) \t[", tid, value);
	const char suffix[] = "]";
	const size_t prefix_length = strlen(prefix);
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(bound + prefix_length + suffix_length + 1, 1);
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < bound; ++i)
	{
	    buffer[prefix_length + i] = i < value/10000 ? '#' : ' ';
	}
	strcpy(&buffer[prefix_length + i], suffix);
        
        if (!(localTid % 7)) 
            printf(ANSI_COLOR_WHITE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 6)) 
            printf(ANSI_COLOR_BLUE "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 5)) 
            printf(ANSI_COLOR_RED "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 4)) 
            printf(ANSI_COLOR_GREEN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 3)) 
            printf(ANSI_COLOR_CYAN "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 2)) 
            printf(ANSI_COLOR_YELLOW "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else if (!(localTid % 1)) 
            printf(ANSI_COLOR_MAGENTA "\b%c[2K\r%s\n" ANSI_COLOR_RESET, 27, buffer);  
        else
            printf("\b%c[2K\r%s\n", 27, buffer);

	fflush(stdout);
	free(buffer);
}

/****************************************************************************************************************
**
**      END DATA FOR PRINT_PROGRESS
**
****************************************************************************************************************/

void* arraySum(void* vptr)
{
    // Cast input pointer back into thread data pointer
    thread_data_t* tdata = (thread_data_t*)vptr;
    long long int threadSum = 0;
    struct timespec start, end;
    long latency;
    while(1)
    {
        long max_latency = -1;
        for(int i = 0; i < tdata->numVals; i++)
        {
            
            clock_gettime(CLOCK_REALTIME, &start);
            threadSum += tdata->data[i];

            // LOCK MUTEX
            pthread_mutex_lock(tdata->lock);

            // MODIFY DATA
            tdata->totalSum += threadSum;

            // UNLOCK MUTEX
            pthread_mutex_unlock(tdata->lock);

            // CALCULATE LATENCY
            clock_gettime(CLOCK_REALTIME, &end);
            latency = end.tv_nsec - start.tv_nsec;
            if(latency > max_latency)
            {
                max_latency = latency;
            }
        }
        print_progress(tdata->localTid, max_latency);
    }
}

int main(int argc, char* argv[])
{
    // Input checking
    if(argc != 2)
    {
        printf("Invalid number of CLI's: %d != 2\n", argc);
        return -1;
    }

    int num_threads = atoi(argv[1]);
    printf("num threads: %d\n", num_threads);

    // Var init
    int* arr;
    int size = 2000000;
    arr = (int*)malloc(size * sizeof(int));
    if(arr == NULL)
    {
        printf("Error initialising array\n");
        return -2;
    }
    
    long long int totalSum = 0;
    
    pthread_mutex_t mut;
    int mutCheck = pthread_mutex_init(&mut, NULL);
    if(mutCheck != 0)
    {
        printf("Error initialising mutex\n");
        return -3;
    }
    
    // Init thread data arr
    thread_data_t* thread_data_arr = (thread_data_t*)malloc(num_threads * sizeof(thread_data_t));
    for(int i = 0; i < num_threads; i++)
    {
        thread_data_arr[i].localTid = i;
        thread_data_arr[i].data = arr;
        thread_data_arr[i].numVals = size;
        thread_data_arr[i].lock = &mut;
        thread_data_arr[i].totalSum = &totalSum;
    }

    printf("Here!\n");
    // Init thread arr
    pthread_t* thread_arr = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    for(int i = 0; i < num_threads; i++)
    {
        pthread_create(&thread_arr[i], NULL, arraySum, &thread_data_arr[i]);
    }
    for(int i = 0; i < num_threads; i++)
    {
        pthread_join(thread_arr[i], NULL);
    }
    
    free(arr);
    arr = NULL;
    printf("End!\n");
    return 0;
}