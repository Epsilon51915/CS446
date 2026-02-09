#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>

int parseInput(char * input, char splitWords[][500], int maxWords);
int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2);
void changeDirectories(const char * path);

int main()
{
    // Vars
    char cwd[500];  
    char input[500];
    char splitWords[10][500];
    char* command1 = NULL;
    char* command2 = NULL;
    char* infile = NULL;
    char* outfile = NULL;

    // I/O
    while(1)
    {
        getcwd(cwd, 500);
        printf("tcarroll:%s$", cwd);
        fgets(input, 500, stdin);

        int numWords = parseInput(input, splitWords, 10);

        // CD and Exit commands which require no parsing

        if(strcmp(splitWords[0], "cd"))
        {
            changeDirectories(splitWords[1]);
        }
        else if(strcmp(splitWords[0], "exit"))
        {
            return 0;
        }
        else
        {
            for(int i = 0; i < 10; i++)
            {
                if(strcmp(splitWords[i], "<"))
                {
                    strcpy(infile, splitWords[i+1]);
                    numWords = numWords -2;
                }
                else if(strcmp(splitWords[i], ">"))
                {
                    strcpy(outfile, splitWords[i+1]);
                    numWords = numWords -2;
                }
                else if(strcmp(splitWords[i], "|"))
                {

                }
            }
            // command1 = input;


            // Dynamically allocate memory

            char** dynamic = malloc((numWords + 1) * sizeof(char *));
            for(int i = 0; i < numWords; i++)
            {
                size_t len = strlen(splitWords[i]) + 1;
                dynamic[i] = malloc(len * sizeof(char));
                strcpy(dynamic[i], splitWords[i]);
            }
            dynamic[numWords] = NULL;
            
            
            // Free memory after usage
            
            for (int i = 0; i < numWords; i++) 
            {
                free(dynamic[i]);
            }
            free(dynamic);
            dynamic = NULL;
            
        }
    
            /*
            int fork_retval = fork();
            if (fork_retval == 0) 
            {
                // CHILD PROCESS
            } 
            else 
            {
                // PARENT PROCESS
            }
            */
    }
    
}

int parseInput(char *input, char splitWords[][500], int maxWords)
{
    char* delim = " ";
    char* token;
    token = strtok(input, delim);
    int words = 0;

    while(token != NULL && words < maxWords)
    {
        printf("%s\n", token);
        strcpy(splitWords[words], token);
        token = strtok(NULL, delim);
        words++;
    }

    return words;
}

int executeCommand(char * const* command, const char* infile, const char* outfile, char * const* command_2)
{
    return 1;
}

void changeDirectories(const char * path)
{
    if(chdir(path) == 0)
    {

    }
    else
    {
        printf(strerror(errno));
    }
}