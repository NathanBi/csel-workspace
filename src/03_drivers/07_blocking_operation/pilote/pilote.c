#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    //Open /dev/mymodule
    int pf = open("/dev/mymodule", O_RDWR);

    //Create the file descriptor
    fd_set set;
    FD_ZERO(&set);

    int NbInterrupts = 0;

    //Infinite loop
    while (1) 
    {
        //Set the file descriptor to /dev/mymodule
        FD_SET(pf, &set);
        int status = select(pf + 1, &set, 0, 0, 0);
        if (status == -1) 
            printf("error while waiting on signal...\n");

        //Test if we have an interrupt
        if (FD_ISSET(pf, &set)) 
        {
            NbInterrupts++;
            printf("Nombre d'interruption : %d\n", NbInterrupts);
        }
        
    }

    //Close /dev/mymodule
    close(pf);
    
    return 0;
}