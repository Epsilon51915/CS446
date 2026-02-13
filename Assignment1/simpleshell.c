#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/wait.h>


int parseInput(char * input, char splitWords[][500], int maxWords);
int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2);
void changeDirectories(const char * path);

int main()
{
    // Vars
    char cwd[500];  
    char input[500];
    char splitWords[10][500];
    char *infile = NULL;
    char *outfile = NULL;
    int pipe = 0;
    char** dynamic = NULL;
    int lenDynamic = 0;
    int lenDynamic2 = 0;
    char** dynamic2 = NULL;
    int malloc_flag = 0;

    // I/O
    while(1)
    {
        getcwd(cwd, 500);
        printf("tcarroll:%s$", cwd);
        fgets(input, 500, stdin);

        int numWords = parseInput(input, splitWords, 10);
        int allocWords = numWords;
        // CD and Exit commands which require no parsing

        if(!strcmp(splitWords[0], "cd"))
        {
            if(numWords != 2)
            {
                printf("Path not formatted correctly!\n");
            }
            else
            {
                changeDirectories(splitWords[1]);
            }
        }
        else if(!strcmp(splitWords[0], "exit"))
        {
            //printf("Exit");
            return 0;
        }
        else
        {
            for(int i = 0; i < numWords; i++)
            {
                //printf("Compared string: %s\n", splitWords[i]);
                if(!strcmp(splitWords[i], "<"))
                {
                    int strleng = strlen(splitWords[i+1]) + 1;
                    infile = malloc(sizeof(*infile) * strleng);
                    strcpy(infile, splitWords[i+1]);
                    allocWords = numWords - 2;
                    malloc_flag = 1;
                }
                else if(!strcmp(splitWords[i], ">"))
                {
                    int strleng = strlen(splitWords[i+1]) + 1;
                    outfile = malloc(sizeof(*infile) * strleng);
                    strcpy(outfile, splitWords[i+1]);
                    allocWords = numWords -2;
                    malloc_flag = 2;
                }
                else if(!strcmp(splitWords[i], "|"))
                {
                    printf("Pipe malloc \n");
                    pipe = 1;
                    dynamic = malloc((i + 1) * sizeof(char *));
                    lenDynamic = i + 1;
                    for(int j = 0; j < i; j++)
                    {
                        size_t len = strlen(splitWords[j]) + 1;
                        dynamic[j] = malloc(len * sizeof(char));
                        strcpy(dynamic[j], splitWords[j]);
                    }
                    dynamic[i] = NULL;

                    dynamic2 = malloc((numWords - i) * sizeof(char *));
                    lenDynamic2 = numWords - i;
                    for(int j = i+1; j < numWords; j++)
                    {
                        size_t len = strlen(splitWords[j]) + 1;
                        dynamic2[j - i - 1] = malloc(len * sizeof(char));
                        strcpy(dynamic2[j - i - 1], splitWords[j]);
                    }
                    dynamic2[numWords - i] = NULL;
                }
            }

            // Dynamically allocate memory
            if(pipe != 1)
            {
                // IF pipe was needed, allocation already happened
               
                // Malloc space for dynamic, we want numWords + 1 string pointers (numWords pointers for command/args, 1 extra for NULL)
                dynamic = malloc((allocWords + 1) * sizeof(char *));
                lenDynamic = allocWords + 1;

                // For each element in dynamic:
                for(int i = 0; i < allocWords; i++)
                {
                    // Find the character count of the word we're putting into the array
                    size_t len = strlen(splitWords[i]) + 1;

                    // Allocate enough space for entire word
                    dynamic[i] = malloc(len * sizeof(char));

                    // Copy the word from splitWords into dynamic
                    strcpy(dynamic[i], splitWords[i]);
                }

                // Finally, the last pointer in dynamic should be NULL.
                dynamic[allocWords] = NULL;
            }
                     
            executeCommand(dynamic, infile, outfile, dynamic2);
            //printf("Ececute finish\n");
            // Free memory after usage
            
            for (int i = 0; i < lenDynamic; i++) 
            {
                //printf("Command 1: %s", dynamic[i]);
                free(dynamic[i]);
                dynamic[i] = NULL;
            }
            for(int i = 0; i < lenDynamic2; i++)
            {
                //printf("Command 2: %s", dynamic2[i]);
                free(dynamic2[i]);
                dynamic2[i] = NULL;
            }

            if(malloc_flag == 1)
            {
                free(infile);
                infile = NULL;
            }
            if(malloc_flag == 2)
            {
                free(outfile);
                outfile = NULL;
            }

            free(dynamic);
            free(dynamic2);
            dynamic = NULL;
            dynamic2 = NULL;
            
            lenDynamic = 0;
            lenDynamic2 = 0;
            pipe = 0;
            malloc_flag = 0;
        }
    }
    
}

int parseInput(char *input, char splitWords[][500], int maxWords)
{
    char delims[] = " \n";
    char* token;
    token = strtok(input, delims);
    int words = 0;

    while(token != NULL && words < maxWords && strcmp(token, "\n"))
    {
        //printf("Word: %s", token);
        strcpy(splitWords[words], token);
        token = strtok(NULL, delims);
        words++;
    }
    return words;
}

int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2)
{
    int pipe_fds[2];
    
    if(command_2 != NULL)
    {
        // Pipe required
        // pipe_fds[0] is read from
        // pipe_fds[1] is write to
        pipe(pipe_fds);

        pid_t pid1 = fork();
        if(pid1 == -1)
        {
            // Error forking
            //int error = *errno();
            char* fail = strerror(errno);
            printf("Fork failed: %s\n", fail);
            return -2;
        }
        else if(pid1 == 0)
        {
            // Child

            // Pipe setup to change stdout to pipe_fds[1]
            dup2(pipe_fds[1], STDOUT_FILENO); //STDOUT_FILENO is 1
            close(pipe_fds[0]); //don’t keep around extra reference to opened pipe_fds
            close(pipe_fds[1]); //don’t keep around extra reference to opened pipe_fds  
                    
             // Standard procedure
             execvp(command[0], command);
             char* fail = strerror(errno);
             printf("Exec failed: %s\n", fail);
             _exit(1); 
        }
        else
        {
            // Parent of child 1, no need to wait
            waitpid(pid1, 0, 0);
        }
        pid_t pid2 = fork();
        if(pid2 == -1)
        {
            // Error forking
            char* fail = strerror(errno);
            printf("Fork failed: %s\n", fail);
            return -2;
        }
        else if(pid2 == 0)
        {
            // Child

            // Pipe setup to change stdout to pipe_fds[1]
            dup2(pipe_fds[0], STDIN_FILENO); //STDOUT_FILENO is 1
            close(pipe_fds[0]); //don’t keep around extra reference to opened pipe_fds
            close(pipe_fds[1]); //don’t keep around extra reference to opened pipe_fds  
                    
             // Standard procedure
             execvp(command_2[0], command_2);
             char* fail = strerror(errno);
             printf("Exec failed: %s\n", fail);
             _exit(1); 
        }
        else
        {
            // Parent of child 2, must wait
            close(pipe_fds[0]);
            close(pipe_fds[1]);
            pid_t err2 = waitpid(pid2, 0, 0);
            printf("Child finished with error code: %d\n", err2);
            return err2;
        }
                
    }
    else
    {
        int pid = fork();
        if(pid == -1)
        {
            // Error forking
            char* fail = strerror(errno);
            printf("Fork failed: %s\n", fail);
            return -2;
        }
        else if(pid == 0)
        {
            // Child
            printf("Child");
            int fd_in = 0;
            if(infile != NULL)
            {
                printf("Infile");
                // Input redirection
                if((fd_in = open(infile, O_RDONLY, 0666)) < 0)
                {
                    perror(infile);
                    _exit(1);
                }
                if(fd_in != 0)
                {
                    // Changed stdin to file
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }
            }
            else if(outfile != NULL)
            {
                // Output redirection
                printf("Outfile");
                if((fd_in = open(outfile, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0)
                {
                    perror(outfile);
                    _exit(1);
                }
                if(fd_in != 0)
                {
                    // Changed stdout to file
                    dup2(fd_in, STDOUT_FILENO);
                    close(fd_in);
                }
            }

             // Standard procedure
             execvp(command[0], command);
             char* fail = strerror(errno);
             printf("Exec failed: %s\n", fail);
             _exit(1);
            
        }
        else
        {
            // Parent
            //printf("Parent of %d\n", pid);
            pid_t err = wait(NULL);
            printf("Child finished with error code: %d\n", err);
            return err;
        }
    }
    return -1;
}

void changeDirectories(const char * path)
{
    if(chdir(path) == 0)
    {
        // Successful chdir
        return;
    }
    else
    {
        char* fail = strerror(errno);
        printf("chdir Failed: %s\n", fail);
        return;
    }
}