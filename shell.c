#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <signal.h>
#include "llist.h"

#define MAX_CMD_SIZE 1024
#define TRUE 1

//Declaring all global variables
int process(char **args);
char promptName[MAX_CMD_SIZE] = "hello:"; //prompt name
int lastCommandStatus = -1; //status
char prevCommand[MAX_CMD_SIZE]; //Previous command buffer
char tmpCommand[MAX_CMD_SIZE]; //Curret command buffer
//Lists for arrows & variables
List commandsMemmory;
List variables;

int status = 0; //status
int number_of_pipes = 0, PIPE_WRITER = 1, PIPE_READER = 0, counter = 0; //pipe variables
int fildes[2];
char *argv[MAX_CMD_SIZE];
int stdoutfd;
pid_t runningProcces = -1;

//Our 'Hash map'
typedef struct Var
{
    char *key;
    char *value;
} Var;
//Search inside the "Hash map"
char *searchVar(char *key)
{
    Node *h = variables.head;
    while (h)
    {
        if (!strcmp(((Var *)h->data)->key, key))
        {
            return ((Var *)h->data)->value;
        }
        h = h->next;
    }
    return NULL;
}
//Gives us an indication of how many commands there are (equal to counting spaces)
int numberOfCommands(char *str)
{
    if (!*str){return 0;}
    int cnt = 0;
    while (*str){
        if (*str == ' '){cnt++;}
        str++;
    }
    return cnt + 1;
}
//Change current directory function
void changeCurrentDir(char *path)
{
    if (chdir(path) != 0)
    {
        printf("chdir() to %s failed\n", path);
        return;
    }
    printf("chdir() to %s\n", path);
}
//If we get a SIGINT signal we need to kill the runningProccess else just type "Ctrl C ijested"
void ctrlCHandler(int sig)
{
    if (runningProcces != -1)
    { // default behavior
        kill(runningProcces, SIGKILL);
        printf("You typed Control-C! A proccess was killed\n");
    }
    else
    {
        printf("You typed Control-C!\n");
    }
}
//Understand what kind of a redirection we have here
int handleRedirection(char **argv, char **outfile, int size)
{
    if (size >= 2 && (!strcmp(argv[size - 2], ">") || !strcmp(argv[size - 2], ">>"))){
        *outfile = argv[size - 1];
        return STDOUT_FILENO;
    }
    else if (size >= 2 && !strcmp(argv[size - 2], "2>")){
        *outfile = argv[size - 1];
        return STDERR_FILENO;
    }
    else if (size >= 2 && !strcmp(argv[size - 2], "<")){
        *outfile = argv[size - 1];
        return STDIN_FILENO;
    }
    return -1;
}
//Printing function to stderr
void printArgs(char **args)
{
    char **p = args;
    while (*p != NULL){
        fprintf(stderr, "%s ", *p);
        p++;
    }
    fprintf(stderr, "\n");
}
//Tokens to split a command ("ls -l -r" => {ls, -l, -r})
void splitCommand(char *command)
{
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL){
        argv[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    argv[i] = NULL;
}
//Return what comes after the pipe
char **findPipeCommand(char **args)
{
    char **p = args;
    while (*p != NULL){
        if (strcmp(*p, "|") == 0){return p;}
        p++;
    }
    return NULL;
}
//Count the length of the command
int argsCount(char **args)
{
    char **p = args;
    int cnt = 0;
    while (*p != NULL){
        p++;
        cnt++;
    }
    return cnt;
}
//Main execution command
int execute(char **args)
{
    char *outfile;
    int i = argsCount(args), fd, amper, redirect = -1, rv = -1;
    pid_t pid;
    int hasPip = 0;
    // find the first pipe sign.
    char **pipPointer = findPipeCommand(args); // returns pointer to the location of the character in the string,NULL otherwise.
    int pipe_fd[2];

    // if there's a pipe use in the command
    if (pipPointer != NULL){
        hasPip = 1;
        *pipPointer = NULL;
        i = argsCount(args);
        pipe(pipe_fd);

        if (fork() == 0){
            close(pipe_fd[PIPE_WRITER]);
            close(STDIN_FILENO);
            dup(pipe_fd[PIPE_READER]);
            execute(pipPointer + 1);
            exit(0);
        }

        stdoutfd = dup(STDOUT_FILENO);
        dup2(pipe_fd[PIPE_WRITER], STDOUT_FILENO);
    }

    /* Is command empty */
    if (args[0] == NULL)
        return 0;
    /* Run the last command */
    if (!strcmp(args[0], "!!")){
        strcpy(tmpCommand, prevCommand);
        splitCommand(tmpCommand);
        execute(argv);
        return 0;
    }
    /* Variable decelration */
    if (args[0][0] == '$' && i >= 3){
        Var *var = (Var *)malloc(sizeof(Var));
        var->key = malloc((strlen(args[0]) + 1));
        var->value = malloc((strlen(args[2]) + 1));
        strcpy(var->key, args[0]);
        strcpy(var->value, args[2]);
        add(&variables, var);
        return 0;
    }

    if(!strncmp(args[0], "if", 2)){
        int j = 1;
        while (argv[j] != NULL) {
            argv[j-1] = argv[j];
            j++;
        }
        argv[j-1] = NULL;
        int currstatus = execute(args);

        char insidestatement[MAX_CMD_SIZE];
        if(!currstatus){
            fgets(insidestatement, 1024, stdin);
            insidestatement[strlen(insidestatement) - 1] = '\0';
            if (!strcmp(insidestatement,"then")){
                fgets(insidestatement, 1024, stdin);
                insidestatement[strlen(insidestatement) - 1] = '\0';
                int elseFlag = 1;
                while(strcmp(insidestatement, "fi")){
                    if(!strcmp(insidestatement, "else")){
                        elseFlag = 0;
                    }
                    if(elseFlag){
                        splitCommand(insidestatement);
                        process(argv);
                    }
                    fgets(insidestatement, 1024, stdin);
                    insidestatement[strlen(insidestatement) - 1] = '\0';
                }
            }
            else{
                printf("Bad if statemnt");
                return 0;
            }
        }
        else{
            fgets(insidestatement, 1024, stdin);
            insidestatement[strlen(insidestatement) - 1] = '\0';
            while(strcmp(insidestatement, "else")){
                    fgets(insidestatement, 1024, stdin);
                    insidestatement[strlen(insidestatement) - 1] = '\0';
                }
            while(strcmp(insidestatement, "fi")){
                    splitCommand(insidestatement);
                    process(argv);
                    fgets(insidestatement, 1024, stdin);
                    insidestatement[strlen(insidestatement) - 1] = '\0';
                }
        }
        return 0;
    }
    /* Read variable decleration */
    if (!strcmp(args[0], "read")){
        Var *var = (Var *)malloc(sizeof(Var));
        var->key = malloc(sizeof(char) * (strlen(args[1])));
        var->value = malloc(sizeof(char) * MAX_CMD_SIZE);
        var->key[0] = '$';
        memset(var->value, 0, MAX_CMD_SIZE);
        strcpy(var->key + 1, args[1]);
        fgets(var->value, MAX_CMD_SIZE, stdin);
        var->value[strlen(var->value) - 1] = '\0';
        add(&variables, var);
        return 0;
    }
    /* Simple cd <folder>*/
    if (!strcmp(args[0], "cd")){
        changeCurrentDir(args[1]);
        return 0;
    }
    /* Change prompt name */
    if (!strcmp(args[0], "prompt")){
        char newPromptName[MAX_CMD_SIZE] = "";
        for (int k = 2; k < i; ++k) {
            strcat(newPromptName, args[k]);
            strcat(newPromptName, " ");
            }
        strcpy(promptName,newPromptName);
        printf("Changed prompt name to %s\n",promptName);
        return 0;
    }
    /* All echo options (variables, $?, regular echo) */
    if (!strcmp(args[0], "echo")){
        char **echo_args = args + 1;
        if (!strcmp(*echo_args, "$?")){
            printf("%d\n", status);
            return 0;
        }
        while (*echo_args){
            if (*echo_args && *echo_args[0] == '$')
            {
                char *v = searchVar(*echo_args);
                if (v != NULL)
                    printf("%s ", v);
            }
            else
                printf("%s ", *echo_args);
            echo_args++;
        }
        printf("\n");
        return 0;
    }

    /* Does command line end with & - if so change the amper(amprecent variable &) */
    if (!strcmp(args[i - 1], "&")){
        amper = 1;
        args[i - 1] = NULL;
    }
    else
        amper = 0;
    int redirectFd = handleRedirection(args, &outfile, i);
    /* for commands not part of the shell command language */

    if ((runningProcces = fork()) == 0){
        /* redirection of IO ? */
        if (redirectFd >= 0)
        {
            if (!strcmp(args[i - 2], ">>")){
                fd = open(outfile, O_WRONLY | O_CREAT);
                lseek(fd, 0, SEEK_END);
            }
            else if (!strcmp(args[i - 2], ">") || !strcmp(args[i - 2], "2>")){
                fd = creat(outfile, 0660);
            }
            else{
                // stdin
                fd = open(outfile, O_RDONLY);
            }

            close(redirectFd);
            dup(fd);
            close(fd);
            /* stdout is now redirected */
            args[i - 2] = NULL;
        }

        execvp(args[0], args);
    }
    /* parent continues here */
    if (amper == 0){
        wait(&status);
        rv = status;
        runningProcces = -1;
    }

    if (hasPip){
        close(STDOUT_FILENO);
        close(pipe_fd[PIPE_WRITER]);
        dup(stdoutfd);
        wait(NULL);
    }

    return rv;
}

int process(char **args)
{
    int rv = -1;
    // do control command
    if (args[0] == NULL){rv = 0;}
    else{rv = execute(args);}
    return rv;
}

int main()
{
    signal(SIGINT, ctrlCHandler);
    char command[MAX_CMD_SIZE];
    int commandPosition = -1;
    char *b;
    int i;
    char ch;
    while(TRUE){
        printf("%s ", promptName);
        ch = getchar();
        if (ch == '\033') //up or down was pressed
        {
            printf("\033[1A"); // line up
            printf("\x1b[2K"); // delete line
            getchar();         // skip the [
            switch (getchar()){
            case 'A':
                // code for arrow up
                if (commandPosition > 0){
                    commandPosition--;
                }
                printf("%s\n", (char *)get(&commandsMemmory, commandPosition));
                break;
            case 'B':
                // code for arrow down
                if (commandPosition < commandsMemmory.size - 1){
                    commandPosition++;
                }
                printf("%s\n", (char *)get(&commandsMemmory, commandPosition));
                break;
            }
            getchar();
            continue;
        }
        if (ch == '\n'){
            splitCommand((char *)get(&commandsMemmory, commandPosition));
            execute(argv);
        }

        command[0] = ch;
        fgets(command + 1, 1023, stdin);
        command[strlen(command) - 1] = '\0';

        // exit
        if (!strcmp(command, "quit")){
            printf("Quit button detected, Thanks for using hello Shell\n");
            exit(0);}

        // save last command
        if (strcmp(command, "!!"))
            strcpy(prevCommand, command);

        // add to commands memory list
        b = malloc(sizeof(char) * strlen(command));
        strcpy(b, command);
        add(&commandsMemmory, b);

        // update last command index
        commandPosition = commandsMemmory.size;

        splitCommand(command);

        // handle command
        status = process(argv);

        // printf("status: %d\n", status);
    }
}
