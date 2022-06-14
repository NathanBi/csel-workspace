#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv)
{

    int mode, frequence, temp;
    int fd;
    char buf[20];
    char b[10];

    if(argc == 3)
    {
        mode = atoi(argv[1]);
        frequence = atoi(argv[2]);

        printf("Mode : %d & Frequence : %d\n", mode, frequence);

        if(frequence > 0 && (mode == 0 || mode == 1))
        {
            printf("Envoie a l'interface IPC\n");

            fd = open("/tmp/myfifo", O_WRONLY);

            if(fd < 0)
            {
                printf("Erreur ouverture fifo\n");
            }
            else
            {
                snprintf(buf,20,"%d %d",mode, frequence);

                write(fd, buf, strlen(buf)+1);
                close(fd);

                printf("Envoie OK\n");
            }
        }
        else
        {
            printf("Erreur valeur parametre\n");
        }
    }
    else
    {
        printf("Erreur nombre parametre\n");
    }

    return 1;
}