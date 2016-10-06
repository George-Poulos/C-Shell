#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>

#define KILLVAL -1

void sig_handler(int sig){
	if(sig == SIGINT){
		signal(SIGINT,sig_handler);
		printf("SIGINT handled.\n");
		fflush(stdout);
	}
	if(sig == SIGTSTP){
		signal(SIGTSTP,sig_handler);
        	printf("SIGTSP handled.\n");
        	fflush(stdout);
	}
}

int run_process ( char *command , char ** args  , int files , char * inputFile , char * outputFile )
{
    int pid = fork();
    printf("PID: %d\n",pid);
    int status = 0;
    if (pid != 0)
    {
        waitpid (pid , &status , WUNTRACED);
        return 0;
    }
    else
    {
        int fd_in = 0;
        int fd_out = 1;
        switch (files & 1)
        {
            case 0:
                break;
            case 1:
                if (inputFile == NULL)
                {
                    fprintf(stderr,"This file is Invalid! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                if (access ( inputFile , F_OK ))
                {
                    fprintf(stderr,"This file is Invalid! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                fd_in = open ( inputFile , O_RDWR );
                if (fd_in == -1)
                {
                    fprintf(stderr, "Wrong Permissions! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                dup2(fd_in , STDIN_FILENO);
                close (fd_in);
                break;
        }
        switch ((files & 2) >> 1)
        {
            case 0:
                break;
            case 1:
                if (outputFile == NULL)
                {
                    fprintf(stderr,"This file is Invalid! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                fd_out = open ( outputFile , O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWGRP | S_IWOTH );
                if (fd_out == -1)
                {
                    fprintf(stderr , "Wrong Permissions! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                dup2(fd_out , STDOUT_FILENO);
                close (fd_out);
                break;
        }
        switch ((files & 4) >> 2)
        {
            case 0:
                break;
            case 1:
                if (outputFile == NULL)
                {
                    fprintf(stderr , "This file is Invalid! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                fd_out = open ( outputFile , O_RDWR | O_CREAT | O_APPEND , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWGRP | S_IWOTH );
                if (fd_out == -1)
                {
                    fprintf(stderr , "Wrong Permissions! \n");
                    kill (getpid() , SIGTERM);
                    return KILLVAL;
                }
                dup2(fd_out , STDOUT_FILENO);
                close (fd_out);
                break;
        }
        if (execvp ( command , args ) < 0)
        {
            fprintf ( stderr , "Command is invalid : %s\n" , command );
            kill (getpid() , SIGTERM);
            return KILLVAL;
        }
    }
    return 0;
}

void parseLine ( char * line, char ** argv )
{
    while (*line != '\0' && *line != '>' && *line != '<')
    {
        while (*line == ' ' || *line == '\t' || *line == '\n')
            *line++ = '\0';
        *argv++ = line;
        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
            line++;
    }
    *argv = '\0';
}

char * readLine ( char *buffer )
{
    char *ch = buffer;
    while ((*ch = getchar()) != '\n')
        ch++;
    *ch = 0;
    return buffer;
}

void parseCommandLine ( char *command )
{
    char *args[100];
    char * inputFile = NULL;
    char * outputFile = NULL;
    parseLine(command , args);
    int flags = 0;
    int len;
    for (len = 0 ; args[len] ; len++)
        ;
    int i;
    for (i = 0 ; args[i] ; i++)
    {
        if (args[i][0] == '<')
        {
            if (i + 1 < len)
                inputFile = args[i + 1];
            else
                inputFile = NULL;
            flags |= 1;
            args[i] = NULL;
            i++;
        }
        if (args[i][0] == '>')
        {
            flags |= 2;
            if (strlen(args[i]) == 2 && args[i][1] == '>')
            {
                flags |= 4;
                flags &= ~2;
            }
            if (i + 1 < len)
                outputFile = args[i + 1];
            else
                outputFile = NULL;
            args[i] = NULL;
            i++;
        }
    }
    int exit = run_process ( command , args , flags , inputFile , outputFile );
    printf("Exit: %d \n",exit);
}

int main ( )
{
    signal(SIGINT, sig_handler);
    signal(SIGTSTP,sig_handler);
    char command[1000];
    while (1)
    {
        printf("361> ");
        readLine(command);
        while (command[0] == '\0')
        {
            printf("361> ");
            readLine(command);
        }
        if (strncmp (command , "quit" , 4) == 0)
            return 0;
        if (strncmp (command , "exit" , 4) == 0)
            return 0;
        
        parseCommandLine(command);
    }
    return 0;
}
