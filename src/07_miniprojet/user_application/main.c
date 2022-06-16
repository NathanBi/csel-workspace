#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char** argv)
{

    int mode, freq;
    int fd;
    char buf[20];

    if(argc == 3)
    {
        mode = atoi(argv[1]);
        freq = atoi(argv[2]);

        printf("Mode : %d & Frequency : %d\n", mode, freq);

        if(freq > 0 && (mode == 0 || mode == 1))
        {
            printf("Send to IPC interface\n");

            fd = open("/tmp/myfifo", O_WRONLY);

            if(fd < 0)
            {
                printf("Error opening FIFO\n");
            }
            else
            {
                snprintf(buf,20,"%d %d",mode, freq);

                write(fd, buf, strlen(buf)+1);
                close(fd);

                printf("Sent successfully\n");
            }
        }
        else
        {
            printf("Error parameter value\n");
        }
    }
    else
    {
        printf("Error parameter amount\n");
    }

    return 1;
}