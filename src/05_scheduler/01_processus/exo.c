#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>

void catch_signal (int signo) {
    /* do something... */
    /* to terminate process execution with
    * 1) success: exit(EXIT_SUCCESS);
    * 2) failure: exit(EXIT_FAILURE);
    */
   printf("Signal receive\n");
}

int main()
{
    int fd[2];
    char buf[30];

    struct sigaction act = {.sa_handler = catch_signal,};
    sigaction(SIGHUP, &act, NULL);

    int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    if (err == -1)
        printf("Error socketpair");

    cpu_set_t set;

    CPU_ZERO(&set);

    pid_t pid = fork();

    if (pid == 0)   //Child
    {
        CPU_SET(1, &set);
        sched_setaffinity(0, sizeof(set), &set);

        close(fd[1]);

        for(int i = 0; i < 4; i++)
        {
            if(i == 0)
                write(fd[0], "Coucou", sizeof("Coucou"));
            else if(i == 1)
                write(fd[0], "ca va ?", sizeof("ca va ?"));
            else if(i == 2)
                write(fd[0], "Au revoir", sizeof("Au revoir"));
            else if(i == 3)
                write(fd[0], "exit", sizeof("exit"));

            read(fd[0],&buf,sizeof(buf));
        }

        close(fd[0]);

    }
    else if (pid > 0)   //Parent
    {
        CPU_SET(0, &set);
        sched_setaffinity(0, sizeof(set), &set);

        close(fd[0]);

        for(int i = 0; i < 4; i++)
        {
            read(fd[1],&buf,sizeof(buf));
            printf("%s\n",buf);
            write(fd[1],"OK",sizeof("OK"));
        }

        close(fd[1]);

        printf("Exit program\n");
    }
    else
        printf("Error create child");

    return 0;
}